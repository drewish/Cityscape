//
//  BlockSubdivider.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/6/16.
//
//

#include "BlockSubdivider.h"

#include "FlatShape.h"
#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
#include "GeometryHelpers.h"

#include <CGAL/Arr_observer.h>

using namespace ci;

namespace Cityscape {

class BaseDivider : public CGAL::Arr_observer<Arrangement_2> {
  public :
    enum State {
        ADDING_OUTLINE,
        ADDING_HOLES,
        DIVIDING
    };

    BaseDivider( Arrangement_2& arr, const FlatShapeRef &s )
        : CGAL::Arr_observer<Arrangement_2>( arr ), mShape( s )
    {
        arrangement()->clear();

        arrangement()->unbounded_face()->set_data( true );

        mState = ADDING_OUTLINE;
        std::list<Segment_2> outlineSegments = contiguousSegmentsFrom( mShape->outline().getPoints() );
        insert_empty( *arrangement(), outlineSegments.begin(), outlineSegments.end() );

        mState = ADDING_HOLES;
        std::vector<Segment_2> holeSegments;
        for ( const auto &hole : mShape->holes() ) {
            for ( const auto &segment : contiguousSegmentsFrom( hole.getPoints() ) ) {
                holeSegments.push_back( segment );
            }
        }
        insert( *arrangement(), holeSegments.begin(), holeSegments.end() );
    }

    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld )
    {
        bool isHole;
        if ( mState == ADDING_OUTLINE ) {
            isHole = false;
        }
        else if ( mState == ADDING_HOLES ) {
            isHole = true;
        }
        else {
            isHole = oldFace->data();
        }
        newFace->set_data( isHole );
    }

    virtual void subdivide( const ZoningPlan::BlockOptions &options )
    {
        mState = DIVIDING;
    }

    virtual void extractLots( BlockRef &block ) {
        for ( auto face = arrangement()->faces_begin(); face != arrangement()->faces_end(); ++face ) {
            if ( face->is_unbounded() || face->data() ) continue;

            // TODO: Refactor into a flatshape from face method.
            Ccb_halfedge_circulator edge = face->outer_ccb();
            PolyLine2f lotOutline = polyLineFrom( edge );
            PolyLine2fs lotHoles;
            for ( auto hole = face->holes_begin(); hole != face->holes_end(); ++hole ) {
                lotHoles.push_back( polyLineFrom( *hole ) );
            }
            LotRef lot = Lot::create( FlatShape::create( lotOutline, lotHoles ) );

            // Start going around looking for a edge that faces a road.
            Arrangement_2::Ccb_halfedge_circulator cc = edge;
            bool leftStart = false;
            while ( !(leftStart && cc == edge) && (cc->twin()->face()->data() == false) ) {
                ++cc;
                leftStart = true;
            }
            // If we find a road copy all we find until we get back to the
            // start.
            if ( cc->twin()->face()->data() ) {
                do {
                    lot->streetFacingSides.push_back( seg2(
                        vecFrom( cc->source()->point() ),
                        vecFrom( cc->target()->point() )
                    ) );
                } while ( ++cc != edge && cc->twin()->face()->data() == true );
            }

            block->lots.push_back( lot );
        }
    }

  protected:
    State mState = ADDING_OUTLINE;
    const FlatShapeRef mShape;
};



// Does a poor job of implementing the OOB algorithm described in:
// Procedural Generation of Parcels in Urban Modeling
// Carlos A. Vanegas, Tom Kelly, Basil Weber, Jan Halatsch, Daniel G. Aliaga, Pascal Müller
// http://www.twak.co.uk/2011/12/procedural-generation-of-parcels-in.html
class OOBSubdivider : public BaseDivider {
  public :
    using BaseDivider::BaseDivider;

    // This finds where the divider intersects the face and then inserts
    // only those segments. This solves the problem of the overlapping dividers
    // splitting adjacent faces.
    //
    // TODO: Need to benchmark alternative ways of doing this. It looks like
    // CGAL::compute_intersection_points creates its own arrangement so we
    // might be better off just making a copy of our arrangement, inserting the
    // divider, observing the new edges and then inserting those into our
    // original arrangement.
    void applyDivider( const Face_handle &face, seg2 divider )
    {
        // Outline
        std::list<Segment_2> faceSegments;
        Arrangement_2::Ccb_halfedge_circulator cc = face->outer_ccb();
        do {
            faceSegments.push_back( Segment_2( cc->source()->point(), cc->target()->point() ) );
        } while ( ++cc != face->outer_ccb() );

        // Holes
        for ( auto hole = face->holes_begin(); hole != face->holes_end(); ++hole ) {
            Arrangement_2::Ccb_halfedge_circulator cc = *hole;
            do {
                faceSegments.push_back( Segment_2( cc->source()->point(), cc->target()->point() ) );
            } while ( ++cc != *hole );
        }

        // Finally add the divider
        faceSegments.push_back( segmentFrom( divider ) );

        std::list<Segment_2> newEdges;
        findIntersections( faceSegments, newEdges );
        if ( newEdges.size() ) {
            insert( *arrangement(), newEdges.begin(), newEdges.end() );
        }
    }

    virtual void subdivide( const ZoningPlan::BlockOptions &options ) override
    {
        mState = DIVIDING;

        toSplit = std::set<Face_handle>();

        for ( auto face = arrangement()->faces_begin(); face != arrangement()->faces_end(); ++face ) {
            queueFace( face );
        }

        while ( toSplit.size() ) {
            auto face = *toSplit.begin();

            while ( true ) {
                Ccb_halfedge_circulator edge = face->outer_ccb();
                PolyLine2f lotOutline = polyLineFrom( edge );
                float area = lotOutline.calcArea();
                if ( area <= options.lotAreaMax ) {
                    break;
                }
                // TODO: Should randomize the divider location and also check
                // for minimum sizing before we accept the division.
                seg2 divider = oobDivider( lotOutline );
                applyDivider( face, divider );
            }

            toSplit.erase( face );
        }
    }

    // Watch for splits so we can queue the new faces for evaluation.
    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld ) override
    {
        BaseDivider::after_split_face( oldFace, newFace, isHoleInOld );

        if ( mState == DIVIDING ) queueFace( newFace );
    }

    void queueFace( Face_handle face )
    {
        if ( !face->is_unbounded() && !face->data() ) toSplit.insert( face );
    }

    std::set<Face_handle> toSplit;
};



class SkeletonSubdivider : public BaseDivider {
  public :
    using BaseDivider::BaseDivider;

    virtual void subdivide( const ZoningPlan::BlockOptions &options ) override
    {
        mState = DIVIDING;

        // For closed outlines, we need at least 4 points.
        if ( mShape->outline().size() < 4 ) return;

        // Build straight skeleton with holes
        SsPtr skel = CGAL::create_interior_straight_skeleton_2( mShape->polygonWithHoles<InexactK>() );

        std::list<Segment_2> skeletonSegments;

        float dividerAngle = 0;
        float maxLength = 0;

        for ( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
            const auto &currVert = edge->vertex(),     &nextVert = edge->next()->vertex();
            const auto &currPoint = currVert->point(), &nextPoint = nextVert->point();

            if ( currVert->is_skeleton() && nextVert->is_skeleton() ) {
                Segment_2 seg( Point_2( currPoint.x(), currPoint.y() ), Point_2( nextPoint.x(), nextPoint.y() ) );

                // Find the angle of the longest skeleton segment edge.
                vec2 vec = vecFrom( currPoint ) - vecFrom( nextPoint );
                float length = glm::length2( vec );
                if ( length > maxLength ) {
                    // Find the perpendicular angle.
                    dividerAngle = -atan2( vec.y, vec.x );
                    maxLength = length;
                }

                // The skeleton has half edges going both directions for each segment in
                // the skeleton. We only need one so before putting a->b in check that
                // b->a isn't already in there.
                auto isReverseOf = [seg]( const Segment_2 &other ) {
                    return other.source() == seg.target() && other.target() == seg.source();
                };
                if ( none_of( skeletonSegments.begin(), skeletonSegments.end(), isReverseOf ) ) {
                    skeletonSegments.push_back( seg );
                }
            }
        }

        // Rather than two      |  |  |  We want one segment    |  |  |
        // segments forking out |  *  |  going to the outline:  |  *  |
        // to the corners:      | / \ |                         |  |  |
        //                      |/   \|                         |  |  |
        //                      *-----*                         *--*--*

        // Find faces with 3 edges: 1 skeleton and 2 contour
        for ( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
            // Move around the face until we get to an edge with a skeleton
            // (seems to be the second edge).
            Ss::Halfedge_handle skelEdge = face->halfedge();
            do {
                skelEdge = skelEdge->next();
            } while ( !skelEdge->vertex()->is_skeleton() );

            // Bail if we don't have two contour verts followed by the skeleton vert.
            Ss::Halfedge_handle contourA = skelEdge->next();
            Ss::Halfedge_handle contourB = contourA->next();
            if ( !contourA->vertex()->is_contour() ) continue;
            if ( !contourB->vertex()->is_contour() ) continue;
            if ( contourB->next() != skelEdge ) continue;

            // Find point where skeleton vector intersects contour edge.
            // TODO: would be better to extend the skeleton out to the edge and find
            // that interesection point rather than picking the midpoint of the edge.
            auto a = contourA->vertex()->point();
            auto b = contourB->vertex()->point();
            Point_2 adj( ( b.x() + a.x() ) / 2.0, ( b.y() + a.y() ) / 2.0 );

            // Create a segment for the adjusted edge
            auto c = skelEdge->vertex()->point();
            skeletonSegments.push_back( Segment_2( Point_2( c.x(), c.y() ), adj ) );
        }

        insert( *arrangement(), skeletonSegments.begin(), skeletonSegments.end() );

        // TODO: would be good to adjust the dividers to:
        // - create lots in specific size ranges (avoid tiny or mega lots)

        // Then start walking across the outline adding dividers.
        std::vector<Segment_2> dividerSegments = mShape->dividerSegment_2s( dividerAngle, options.lotWidth );

        // Put the adjusted skeleton, and new dividers into the the arrangment.
        insert( *arrangement(), dividerSegments.begin(), dividerSegments.end() );
    }
};

Arrangement_2 _lastArrangement;
const Arrangement_2& lastArrangement() { return _lastArrangement; }

// in Blocks
// out Lots
void subdivideBlocks( CityModel &city )
{
    for ( auto &district : city.districts ) {
        ZoningPlanRef zoning = district->zoningPlan;

        for ( auto &block : district->blocks ) {
            ZoningPlan::LotDivision d = zoning->block.lotDivision;

            // Don't bother dividing small blocks
            if ( block->shape->area() < 100 ) {
                d = ZoningPlan::LotDivision::NO_LOT_DIVISION;
            }

            Arrangement_2 arrangement;
            BaseDivider *obs;
            if ( zoning->block.lotDivision == ZoningPlan::LotDivision::OOB_LOT_DIVISION ) {
                obs = new OOBSubdivider( arrangement, block->shape );
            }
            else if ( zoning->block.lotDivision == ZoningPlan::LotDivision::SKELETON_LOT_DIVISION ) {
                obs = new SkeletonSubdivider( arrangement, block->shape );
            }
            else {
                obs = new BaseDivider( arrangement, block->shape );
            }
            obs->subdivide( zoning->block );
            obs->extractLots( block );
            delete obs;
            _lastArrangement = Arrangement_2( arrangement );
        }
    }
}

} // Cityscape namespace
