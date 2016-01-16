#include "BlockMode.h"
#include "GeometryHelpers.h"

using namespace ci;
using namespace ci::app;


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
            vec2(133,41), // The difference is this point to close the loop
        });
        layout();
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mOutline = PolyLine2f({
            vec2(179.795,607.902),
            vec2(174.921,200.486),
            vec2(405.803,358.815),
            vec2(388.601,79.3861),
            vec2(193.129,146.753),
            vec2(216.225,10.8599),
            vec2(205.467,-41.0754),
            vec2(467.135,-22.5028),
            vec2(516.892,456.916),
            vec2(179.795,607.902),
        });
        layout();
    }, "key=3" );
}

void BlockMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    layout();
}

void BlockMode::layout() {
    mArr.clear();
    if (mOutline.size() < 4) return;

    float angle = 0;

    // Build straight skeleton
    // TODO figure out why we need to reverse this...
    CGAL::Polygon_2<InexactK> poly = polygonFrom<InexactK>( mOutline );
    if ( poly.is_clockwise_oriented() ) {
        poly.reverse_orientation();
    }
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( poly, InexactK() );

    std::list<Segment_2> outlineSegments;
    std::list<Segment_2> skeletonSegments;

    Ss::Halfedge longest = *skel->halfedges_begin();
    float len = 0;

    for( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
        const auto &curr = edge->vertex(), &next = edge->next()->vertex();
        const auto &a1 = curr->point(),    &b1 = next->point();
        Segment_2 seg( Point_2( a1.x(), a1.y() ), Point_2( b1.x(), b1.y() ) );

        // Find the angle of the longest outter edge
        // TODO see if we use the longest edge in the skeleton instead.
        if ( edge->is_border() ) {
            // Use the skeleton's outline since, unlike mOutline, will always
            // be a closed polygon.
            outlineSegments.push_back( seg );

            vec2 v = vecFrom(b1) - vecFrom(a1);
            if (glm::length(v) > len) {
                len = glm::length(v);
                angle = atan2( v.y, v.x );
                longest = *edge;
            }
        }

        // The skeleton has half edges going both directions for each segment in
        // the skeleton. We only need one so before putting a->b in check that
        // b->a isn't already in there.
        auto predicate = [seg](const Segment_2 &other){
            return other.source() == seg.target() && other.target() == seg.source();
        };
        if ( curr->is_skeleton() && next->is_skeleton() ) {
            if ( none_of( skeletonSegments.begin(), skeletonSegments.end(), predicate ) ) {
                skeletonSegments.push_back( seg );
            }
        }
    }

    // Adjust the skeleton so it intersects with the outline rather than doing
    // it's normal split thing.

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

        // Find point where skeleton vector intersects contour edge.
        auto a = contourA->vertex()->point();
        auto b = contourB->vertex()->point();
        auto c = skelEdge->vertex()->point();
        // TODO: figure out how to do the math on the points directly rather
        // than upacking
        Point_2 adj( ( b.x() + a.x() ) / 2.0, ( b.y() + a.y() ) / 2.0 );

        // Create a segment for the adjusted edge
        skeletonSegments.push_back( Segment_2( Point_2( c.x(), c.y() ), adj ) );
    }

    // Then start walking across the outline looking for the intersections...
    std::list<Segment_2> dividerSegments;
    mDividers = computeDividers( mOutline.getPoints(), angle );
    for ( const Segment_2 &divider : segmentsFrom( mDividers ) ) {
        outlineSegments.push_back( divider );

        std::vector<Point_2> dividerPoints;
        CGAL::compute_intersection_points( outlineSegments.begin(), outlineSegments.end(), std::back_inserter(dividerPoints) );
        for ( const Segment_2 &dividerChunk : segmentsFrom( dividerPoints ) ) {
            dividerSegments.push_back( dividerChunk );
        }

        outlineSegments.pop_back();
    }

    // Put the outline and adjusted skeleton into the arrangment followed
    insert_empty( mArr, outlineSegments.begin(), outlineSegments.end() );
    insert( mArr, skeletonSegments.begin(), skeletonSegments.end() );
    insert( mArr, dividerSegments.begin(), dividerSegments.end() );
}

void BlockMode::draw() {
    gl::color( 1, 0, 1 );
    assert( mDividers.size() % 2 == 0 );
    for ( auto i = mDividers.begin(); i != mDividers.end(); ++i) {
        gl::drawLine( *i, *++i );
    }

    gl::color( 1, 1, 0 );
    for ( auto i = mArr.vertices_begin(); i != mArr.vertices_end(); ++i ) {
        gl::drawSolidCircle( vecFrom( i->point() ), 5 );
    }

    gl::color(0, 0, 0 );
    for ( auto i = mArr.edges_begin(); i != mArr.edges_end(); ++i ) {
        PolyLine2f p = PolyLine2f({ vecFrom( i->source()->point() ), vecFrom( i->target()->point() ) } );
        gl::draw( p );
    }

    float steps = 0;
//    std::cout << "\n\n------\nfaces: " << mArr.number_of_faces() << std::endl;
    for ( auto i = mArr.faces_begin(); i != mArr.faces_end(); ++i ) {
//        std::cout << "\tunbounded: " << i->is_unbounded() << " fictitious: " << i->is_fictitious() << std::endl;
//        std::cout << "\touter_ccbs:" << i->number_of_outer_ccbs() << " holes: " << i->number_of_holes() << std::endl;
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
//            std::cout << "\t\t" << num << ": " << faceOutline << "\n";
            gl::drawSolid( faceOutline );
        }
    }
}
