//
//  CgalArrangment.h
//  Cityscape
//
//  Created by Andrew Morton on 7/7/15.
//
//

#pragma once

#include "CgalKernel.h"
#include "CgalPolygon.h"

#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arr_observer.h>
#include <CGAL/Arr_default_overlay_traits.h>
#include <CGAL/Arr_overlay_2.h>


enum class FaceRole { Hole, Shape };
enum class EdgeRole { Undef = 0, Exterior, Divider };
typedef CGAL::Arr_segment_traits_2<ExactK>              Traits_2;
// vert, halfedge, face but I'm only using edge and face values
typedef CGAL::Arr_extended_dcel<Traits_2, float, EdgeRole, FaceRole> Dcel;
typedef CGAL::Arrangement_2<Traits_2, Dcel>             Arrangement_2;
typedef Traits_2::Point_2                               Point_2;
typedef Traits_2::X_monotone_curve_2                    Segment_2;
typedef CGAL::Arr_naive_point_location<Arrangement_2>   Naive_pl;


inline ci::vec3 vec3From( const Arrangement_2::Vertex_const_handle &vertex )
{
    return ci::vec3( vecFrom( vertex->point() ), vertex->data() );
}

/**
 * These classes track an arrangement, noting holes, and tracks its division. When
 * faces are split they checks if the face was a hole or not and marks the newly
 * split faces accordingly.
 */
struct OutlineObserver : public CGAL::Arr_observer<Arrangement_2> {
    using CGAL::Arr_observer<Arrangement_2>::Arr_observer;

    OutlineObserver( Arrangement_2& arr ) : CGAL::Arr_observer<Arrangement_2>( arr ) {
        arrangement()->unbounded_face()->set_data( FaceRole::Hole );
    };
    virtual void after_attach() {
        arrangement()->unbounded_face()->set_data( FaceRole::Hole );
    }
    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld ) {
        newFace->set_data( FaceRole::Shape );
    }
};

struct HoleObserver : public CGAL::Arr_observer<Arrangement_2> {
    HoleObserver( Arrangement_2& arr ) : CGAL::Arr_observer<Arrangement_2>( arr ) {};
    // in after_split_face mark new faces as holes
    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld ) {
        newFace->set_data( FaceRole::Hole );
    }
};

/**
 * This allow you to overlay two arrangments and it'll combine the edge and face
 * roles we can extract which faces are holes and which edges are external.
 *
 * First arrangement should be the shape, second should be dividers.
 */
struct Arr_extended_overlay_traits : public CGAL::_Arr_default_overlay_traits_base<Arrangement_2, Arrangement_2, Arrangement_2>
{
    typedef typename Arrangement_2::Halfedge_const_handle Halfedge_handle_A;
    typedef typename Arrangement_2::Halfedge_const_handle Halfedge_handle_B;
    typedef typename Arrangement_2::Halfedge_handle       Halfedge_handle_R;
    typedef typename Arrangement_2::Face_const_handle     Face_handle_A;
    typedef typename Arrangement_2::Face_const_handle     Face_handle_B;
    typedef typename Arrangement_2::Face_handle           Face_handle_R;

    // Faces overlap
    virtual void create_face( Face_handle_A f1, Face_handle_B f2, Face_handle_R f ) const {
        // f1 should be the lot/block so just use its value, that way we don't
        // have to worry about setting the face roles on the dividers.
        f->set_data( f1->data() );
        //f->set_data( f1->data() == FaceRole::Hole || f2->data() == FaceRole::Hole ? FaceRole::Hole : FaceRole::Lot );
    }

    // Edges overlap
    virtual void create_edge( Halfedge_handle_A e1, Halfedge_handle_B e2, Halfedge_handle_R e ) const {
        if( e1->data() == EdgeRole::Exterior || e2->data() == EdgeRole::Exterior )
            e->set_data( EdgeRole::Exterior );
        else
            e->set_data( EdgeRole::Divider );
        e->twin()->set_data( e->data() );
    }

    // Edge splits face
    virtual void create_edge( Halfedge_handle_A e1, Face_handle_B f2, Halfedge_handle_R e ) const {
        e->set_data( e1->data() );
        e->twin()->set_data( e1->data() );
    }

    // Edge splits face
    virtual void create_edge( Face_handle_A f1, Halfedge_handle_B e2, Halfedge_handle_R e ) const {
        e->set_data( e2->data() );
        e->twin()->set_data( e2->data() );
    }
};

/**
 * Overlay that tracks edge and hole roles but also applies the divider's height
 * to the arrangement.
 *
 * First arrangement should be the shape, second should be dividers.
 */
struct OverlayWithHeight : public Arr_extended_overlay_traits
{
    typedef typename Arrangement_2::Vertex_const_handle   Vertex_handle_A;
    typedef typename Arrangement_2::Vertex_const_handle   Vertex_handle_B;
    typedef typename Arrangement_2::Vertex_handle         Vertex_handle_R;


    float interpolate( const Arrangement_2::Vertex_const_handle &source, const Point_2 &intersect, const Arrangement_2::Vertex_const_handle &target ) const
    {
        ci::vec2 s = vecFrom( source->point() );
        float distance = sqrt( ci::length2( vecFrom( intersect ) - s ) / ci::length2( vecFrom( target->point() ) - s ) );
        float lerp = ci::lerp( source->data(), target->data(), distance );
        return lerp;
    }

    // Same vert (use height from divider)
    virtual void create_vertex( Vertex_handle_A v1, Vertex_handle_B v2, Vertex_handle_R v ) const
    {
        v->set_data( v2->data() );
    }

    // Vert in middle of edge gets liner interoplation of height.
    virtual void create_vertex( Vertex_handle_A v1, Halfedge_handle_B e2, Vertex_handle_R v ) const
    {
        v->set_data( interpolate( e2->source(), v->point(), e2->target() ) );
    }

    // Vert from shape just carries height over.
    virtual void create_vertex( Vertex_handle_A v1, Face_handle_B /* f2 */, Vertex_handle_R v ) const
    {
        v->set_data( v1->data() );
    }

    // Divider vertex carries its height over.
    virtual void create_vertex( Halfedge_handle_A /* e1 */, Vertex_handle_B v2, Vertex_handle_R v ) const
    {
        v->set_data( v2->data() );
    }

    // Divider vertex carries its height over.
    virtual void create_vertex( Face_handle_A /* f1 */, Vertex_handle_B v2, Vertex_handle_R v ) const
    {
        v->set_data( v2->data() );
    }

    // Interpolate the height where the intersection happens on the divider
    virtual void create_vertex( Halfedge_handle_A e1, Halfedge_handle_B e2, Vertex_handle_R v ) const
    {
        v->set_data( interpolate( e2->source(), v->point(), e2->target() ) );
    }
};



// Sets data on all but the unbounded face.
void setFaceRoles( Arrangement_2 &arr, FaceRole data );
void setEdgeRoles( Arrangement_2 &arr, EdgeRole data );
void setVertexData( Arrangement_2 &arr, float data );

ci::PolyLine2f polyLineFrom( const Arrangement_2::Ccb_halfedge_const_circulator &circulator );
std::vector<ci::vec3> vec3sFrom( const Arrangement_2::Ccb_halfedge_const_circulator &circulator );

inline Point_2 pointFrom( const ci::vec2 &p )
{
    return Point_2( p.x, p.y );
}

typedef std::pair<ci::vec2, ci::vec2> seg2;
inline Segment_2 segmentFrom( const seg2 &s )
{
    return Segment_2( pointFrom( s.first ), pointFrom( s.second ) );
}
inline Segment_2 segmentFrom( const ci::vec2 &a, const ci::vec2 &b )
{
    return Segment_2( pointFrom( a ), pointFrom( b ) );
}

Arrangement_2 arrangementFromOutline( const ci::PolyLine2f &outline );

void findIntersections( const std::list<Segment_2> &input, std::list<Segment_2> &newEdges );

// For an input: a,b,c
//   when open: a->b,b->c
//   when closed: a->b,b->c,c->a
// For an input: a,b,c,a
//   when open: a->b,b->c,c->a
//   when closed: a->b,b->c,c->a
template<class OI>
void contiguousSegmentsFrom( const ci::PolyLine2f &polyline, OI out )
{
    if ( polyline.size() < 2 ) return;

    std::transform( polyline.begin(), polyline.end() - 1, polyline.begin() + 1, out,
        []( const ci::vec2 &a, const ci::vec2 &b ) { return segmentFrom( a, b ); } );

    if ( polyline.isClosed() ) {
        auto &points = polyline.getPoints();
        if ( points.front() != points.back() ) {
            out++ = segmentFrom( points.back(), points.front() );
        }
    }
}
// Segments will be created from a->b, b->c, c->d
template<class OI>
void contiguousSegmentsFrom( const std::vector<ci::vec2> &points, OI out )
{
    if ( points.empty() ) return;

    std::transform( begin( points ), end( points ) - 1, begin( points ) + 1, out,
        []( const ci::vec2 &a, const ci::vec2 &b ) {
            return Segment_2( pointFrom( a ), pointFrom( b ) );
        } );
}
template<class OI>
void contiguousSegmentsFrom( const std::vector<Point_2> &points, OI out )
{
    if ( points.empty() ) return;

    std::transform( begin( points ), end( points ) - 1, begin( points ) + 1, out,
        []( const Point_2 &a, const Point_2 &b ) {
            return Segment_2( a, b );
        } );
}
template<class OI>
void contiguousSegmentsFrom( const CGAL::Polygon_2<InexactK> &polygon, OI out )
{
    if ( polygon.size() < 2 ) return;

    for( auto edge = polygon.edges_begin(); edge != polygon.edges_end(); ++edge ) {
        auto &s = edge->source();
        auto &t = edge->target();
        // This is a tacky amount of type conversion to go from the straight
        // skeleton kernel to the arrangement kernel.
        out++ = Segment_2( Point_2( s.x(), s.y() ), Point_2( t.x(), t.y() ) );
    }
}

// Segments will be created from a->b, c->d
std::list<Segment_2> segmentsFrom( const std::vector<ci::vec2> &points );
std::list<Segment_2> segmentsFrom( const std::vector<Point_2> &points );
