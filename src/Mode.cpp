//
//  Mode.cpp
//  Cityscape
//
//  Created by andrew morton on 6/16/15.
//
//

#include "Mode.h"

using namespace ci;
using namespace ci::app;

BaseMode::BaseMode()
{
    mOptions.buildingShader = ci::gl::GlslProg::create(
        ci::app::loadResource( RES_VERT ),
        ci::app::loadResource( RES_FRAG )
    );
}

// * * *

void CityMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = true;
}

void CityMode::addParams( ci::params::InterfaceGlRef params) {

    // TODO: Don't redo layout on every change, set a timer to update every half
    // second or so.
//    params->addParam( "highwayWidth", &mOptions.road.highwayWidth )
//        .min( 10 ).max( 50 ).step( 1 ).updateFn( std::bind( &CityMode::layout, this ) );
//    params->addParam( "sidestreetWidth", &mOptions.road.sidestreetWidth )
//        .min( 10 ).max( 50 ).step( 1 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "blockWidth", &mOptions.road.blockWidth )
        .min( 15 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "blockHeight", &mOptions.road.blockHeight )
        .min( 15 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();
    
    params->addParam( "Division", {"None", "Divided"}, (int*)&mOptions.block.division )
        .updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "Placement", {"Center", "Fill"}, (int*)&mOptions.lot.buildingPlacement )
        .updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=f" );

    params->addSeparator();

    params->addButton( "Clear Points", [&] {
        mRoads.clear();
        mRoads.layout( mOptions );
    }, "key=0" );
    params->addButton( "Test 1", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(133,41),
            vec2(143,451),
            vec2(143,451),
            vec2(495,424),
            vec2(491,421),
            vec2(370,254),
            vec2(377,262),
            vec2(529,131),
        });
        mRoads.layout( mOptions );
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(133,41),
            vec2(143,451),
            vec2(143,451),
            vec2(495,424),
            vec2(491,421),
            vec2(370,254),
            vec2(377,262),
            vec2(529,131),
            vec2(131,47),
            vec2(523,132),
        });
        mRoads.layout( mOptions );
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mRoads.addPoints({
            vec2(163.104,60.2898),
            vec2(306.353,918.302),
            vec2(306.353,918.302),
            vec2(490.026,113.687),
            vec2(490.026,113.687),
            vec2(163.104,60.2898),
        });
        mRoads.layout( mOptions );
    }, "key=3" );
    params->addButton( "Test 4", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(-391.031,1191.03),
            vec2(1026.58,1173.85),
            vec2(1026.58,1173.85),
            vec2(538.783,-52.5473),
            vec2(538.783,-52.5473),
            vec2(103.206,-48.1886),
            vec2(103.206,-48.1886),
            vec2(-391.031,1191.03),
        });
        mRoads.layout( mOptions );
    }, "key=4" );
    params->addButton( "Test 5", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(0.8666,1108.26),
            vec2(158.236,41.0269),
            vec2(159.075,44.556),
            vec2(313.45,0.94),
            vec2(313.45,0.94),
            vec2(408.625,90.0115),
            vec2(408.625,90.0115),
            vec2(331.941,319.65),
            vec2(331.941,319.65),
            vec2(313.635,1054.66),
            vec2(313.635,1054.66),
            vec2(0.1429,1069.64),
        });
        mRoads.layout( mOptions );
    }, "key=5" );
}

void CityMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mRoads.addPoint( point );
    mRoads.layout( mOptions );
}

void CityMode::layout() {
    mRoads.layout( mOptions );
}

void CityMode::draw() {
    mRoads.draw( mOptions );
}

// * * *

#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
// This is hacky but we only use one instance of it at a time.
Arrangement_2 mArr;
std::vector<vec2> mDividers;

void BlockMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = true;
layout();
}

void BlockMode::addParams( ci::params::InterfaceGlRef params) {
    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=f" );

    params->addButton( "Clear Points", [&] {
        mOutline = PolyLine2f();
        layout();
    }, "key=0");
    params->addButton( "Test 1", [&] {
        mOutline = PolyLine2f({
            vec2(133,41),
            vec2(143,451),
            vec2(495,424),
            vec2(370,254),
            vec2(529,131),
        });
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mOutline = PolyLine2f({
            vec2(133,41),
            vec2(143,451),
            vec2(495,424),
            vec2(370,254),
            vec2(529,131),
            vec2(133,41),
        });
        layout();
    }, "key=2" );
}

void BlockMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    layout();
}

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<vec2> computeDividers( const std::vector<vec2> &outline, const float angle = 0, const float width = 100 )
{
    // Rotate the shape to the desired angle...
    Rectf outlineBounds( outline );
    vec2 center = vec2( outlineBounds.getWidth() / 2.0, outlineBounds.getHeight() / 2.0 );
    glm::mat3 matrix;
    matrix = translate( rotate( translate( matrix, -center ), angle ), center );

    // ...then find the bounding box...
    std::vector<vec2> rotated;
    for( auto it = outline.begin(); it != outline.end(); ++it ) {
        rotated.push_back( vec2( matrix * vec3( *it, 1 ) ) );
    }
    Rectf bounds = Rectf( rotated ).scaledCentered(1.1);

    // ...now figure out where the left edge of that box would be in the
    // unrotated space...
    mat3 reverse = inverse( matrix );
    vec2 topLeft =    vec2( reverse * vec3( bounds.getUpperLeft(), 1 ) );
    vec2 bottomLeft = vec2( reverse * vec3( bounds.getLowerLeft(), 1 ) );
    vec2 direction = normalize( vec2( reverse * ( vec3( 1, 0, 0 ) ) ) );

    // ...and work across from those points finding dividers
    std::vector<vec2> result;
    for ( float distance = width; distance < bounds.getWidth(); distance += width ) {
        vec2 thing = direction * distance;
        result.push_back( thing + topLeft );
        result.push_back( thing + bottomLeft );
    }

    return result;
}

void BlockMode::layout() {
    mArr.clear();
    if (mOutline.size() < 4) return;

    // Put the outline onto the arrangment.
    std::list<Segment_2> outlineSegments;// = contiguousSegmentsFrom( mOutline.getPoints() );
//    insert_empty( mArr, outlineSegments.begin(), outlineSegments.end() );

    std::list<Segment_2> newEdges;
    std::list<Point_2> newPoints;

    float angle = 0;

    // Build straight skeleton
    // TODO figure out why we need to reverse this...
    CGAL::Polygon_2<InexactK> poly = polygonFrom<InexactK>( mOutline );
    if ( poly.is_clockwise_oriented() ) {
        poly.reverse_orientation();
    }
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( poly, InexactK() );

    // Put all the skeleton lines in
    Ss::Halfedge longest = *skel->halfedges_begin();
    float len = 0;

    for( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
        auto curr = edge->vertex(), next = edge->next()->vertex();
        auto a = curr->point(), b = next->point();

        // TODO Figure out how to only one half of an edge into the arrangement
        // (a->b rather than a->b and b->a)
        if ( curr->is_skeleton() && next->is_skeleton() ) {
            outlineSegments.push_back( Segment_2( Point_2( a.x(), a.y() ), Point_2( b.x(), b.y() ) ) );
        }

        if ( edge->is_border() ) {
            outlineSegments.push_back( Segment_2( Point_2( a.x(), a.y() ), Point_2( b.x(), b.y() ) ) );

            // Find the angle of the longest outter edge
            vec2 v = vecFrom(b) - vecFrom(a);
            if (glm::length(v) > len) {
                len = glm::length(v);
                angle = atan2(v.y, v.x);
                longest = *edge;
            }
        }
    }

    // Find faces with 3 edges: 1 skeleton and 2 contour
    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        // Move around the face until we get to an edge with a skeleton
        // (seems to be the second edge).
        Ss::Halfedge_handle skelEdge = face->halfedge();
        do {
            skelEdge = skelEdge->next();
        } while ( !skelEdge->vertex()->is_skeleton() );

        // Bail if we don't have two contour verts followed by the skeleton vert.
        Ss::Halfedge_handle contourA = skelEdge->next();
        Ss::Halfedge_handle contourB = contourA->next();
        if (!contourA->vertex()->is_contour()) continue;
        if (!contourB->vertex()->is_contour()) continue;
        if (contourB->next() != skelEdge) continue;

        // Find point where skeleton vector intersects contour edge
        vec2 A = vecFrom( contourA->vertex()->point() );
        vec2 B = vecFrom( contourB->vertex()->point() );
        vec2 C = vecFrom( skelEdge->vertex()->point() );
        vec2 adjustment =  ( ( B + A ) / vec2( 2.0 ) );

        // Create a segment for the adjusted edge
        outlineSegments.push_back( Segment_2( Point_2( C.x, C.y ), Point_2( adjustment.x, adjustment.y ) ) );
    }
    insert_empty( mArr, outlineSegments.begin(), outlineSegments.end() );

    // ...and a list of segements to intersect with.
    std::list<Segment_2> intersect;
    intersect.insert( intersect.begin(), outlineSegments.begin(), outlineSegments.end() );

    mDividers = computeDividers( mOutline.getPoints(), angle );

    // Then start walking across the outline looking for the intersections...
    auto segs = segmentsFrom( mDividers );
    for ( auto i = segs.begin(); i != segs.end(); ++i ) {
        intersect.push_back( *i );
//        findIntersections( intersect, newEdges, newPoints );
        intersect.pop_back();
    }

    // Add the new edges all at once for better performance.
    if (newEdges.size()) insert( mArr, newEdges.begin(), newEdges.end() );
}

void BlockMode::draw() {
    gl::color(1, 0, 1);
    assert( mDividers.size() % 2 == 0 );
    for ( auto i = mDividers.begin(); i != mDividers.end(); ++i) gl::drawLine( *i, *++i );


    for ( auto i = mArr.vertices_begin(); i != mArr.vertices_end(); ++i ) {
        vec3 v = vec3( vecFrom( i->point() ), 0 );
        gl::drawColorCube( v, vec3( 10 ) );
    }

    gl::color(1, 0, 0 );
    for ( auto i = mArr.edges_begin(); i != mArr.edges_end(); ++i ) {
        PolyLine2f p = PolyLine2f({ vecFrom( i->source()->point() ), vecFrom( i->target()->point() ) } );
        gl::draw( p );
    }

    float steps = 0;
//std::cout << "\n\n------\nfaces: " << mArr.number_of_faces() << std::endl;
    for ( auto i = mArr.faces_begin(); i != mArr.faces_end(); ++i ) {
//std::cout << "\tunbounded: " << i->is_unbounded() << " fictitious: " << i->is_fictitious() << std::endl;
//std::cout << "\touter_ccbs:" << i->number_of_outer_ccbs() << " holes: " << i->number_of_holes() << std::endl;
int num = 0;
/*
        for ( auto j = i->holes_begin(); j != i->holes_end(); ++j ) {
            PolyLine2f faceOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
            do {
                Arrangement_2::Halfedge_handle he = cc;
                faceOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );

            gl::color( ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
            steps += 0.17;
            if (steps > 1) steps -= 1.0;
std::cout << "\t\t" << num << faceOutline << "\n";
            gl::drawSolid( faceOutline );
        }
*/

        for ( auto j = i->outer_ccbs_begin(); j != i->outer_ccbs_end(); ++j ) {
            PolyLine2f faceOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
            faceOutline.push_back( vecFrom( cc->source()->point() ) );
            do {
                Arrangement_2::Halfedge_handle he = cc;
                faceOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );

            gl::color( ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
            steps += 0.27;
            if (steps > 1) steps -= 1.0;
//std::cout << "\t\t" << num << ": " << faceOutline << "\n";
            gl::drawSolid( faceOutline );
        }
/*
        if ( i->number_of_outer_ccbs() ) {
            PolyLine2f outline;

            Arrangement_2::Ccb_halfedge_const_circulator first, curr;
            curr = first = i->outer_ccb();
            if (!curr->source()->is_at_open_boundary())
                std::cout << "(" << curr->source()->point() << ")";
            do {
                Arrangement_2::Halfedge_const_handle he = curr;
//                if (! he−>is_fictitious())
//                    std::cout << "␣␣␣[" << he−>curve() << "]␣␣␣";
//                else
//                    std::cout << "␣␣␣[␣...␣]␣␣␣";
//
                if ( ! he->target()->is_at_open_boundary() ) {
                    outline.push_back( vecFrom( he->target()->point() ) );
                }

            } while (++curr != first);

            gl::color( ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
            steps += 0.17;
            if (steps > 1) steps -= 1.0;
std::cout << "\t\t" << num << outline << "\n";
            gl::drawSolid( outline );

        }
*/
    }
}

// * * *

void BuildingMode::setup() {
    mOptions.drawBuildings = true;
    mOutline = BuildingPlan::lshape();
}

void BuildingMode::addParams( params::InterfaceGlRef params ) {
    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)(&mBuildingRoof) )
        .keyDecr( "[" ).keyIncr( "]" )
        .updateFn( std::bind( &BuildingMode::layout, this ) );
    params->addParam( "Floors", &mFloors)
        .min( 1 ).max( 5 )
        .keyDecr( "-" ).keyIncr( "=" )
        .updateFn( std::bind( &BuildingMode::layout, this ) );
    params->addSeparator();
    params->addButton( "Square", [&] {
        mOutline = BuildingPlan::square();
        layout();
    }, "key=1" );
    params->addButton( "Rect", [&] {
        mOutline = BuildingPlan::rectangle(60, 40);
        layout();
    }, "key=2" );
    params->addButton( "L", [&] {
        mOutline = BuildingPlan::lshape();
        layout();
    }, "key=3" );
    params->addButton( "T", [&] {
        mOutline = BuildingPlan::tee();
        layout();
    }, "key=4" );
    params->addButton( "+", [&] {
        mOutline = BuildingPlan::plus();
        layout();
    }, "key=5" );
    params->addButton( "<", [&] {
        mOutline = BuildingPlan::triangle();
        layout();
    }, "key=6" );
}

void BuildingMode::addPoint( ci::vec2 point ) {
}

void BuildingMode::layout() {
    mBuilding = Building::create( BuildingPlan( mOutline, mBuildingRoof ), mFloors );
    mBuilding->layout( mOptions );
}

void BuildingMode::draw() {
    gl::translate( getWindowCenter() );
    gl::rotate( 2 * M_PI * mMousePos.x / (float) getWindowWidth() );
    gl::scale( 8, 8, 8);

    if ( mBuilding ) mBuilding->draw( mOptions );
}
