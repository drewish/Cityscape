#pragma once
#include "cinder/Vector.h"
#include "cinder/PolyLine.h"
#include "cinder/Rect.h"
#include "cinder/Shape2d.h"

typedef std::pair<ci::vec2, ci::vec2> seg2;


// Segments will be created from a->b, b->c, c->d
template<class OI>
void contiguousSeg2sFrom( const std::vector<ci::vec2> &points, OI out )
{
    if ( points.empty() ) return;

    std::transform( begin( points ), end( points ) - 1, begin( points ) + 1, out,
        []( const ci::vec2 &a, const ci::vec2 &b ) { return seg2( a, b ); } );
}

// For an input: a,b,c
//   when open: a->b,b->c
//   when closed: a->b,b->c,c->a
// For an input: a,b,c,a
//   when open: a->b,b->c,c->a
//   when closed: a->b,b->c,c->a
void pointsInPairs( const ci::PolyLine2f &outline, std::function<void(const ci::vec2&, const ci::vec2&)> process );

// Find the angle in radians between each pair of points.
template<class OI>
void anglesBetweenPointsIn( const ci::PolyLine2f &outline, OI out )
{
    pointsInPairs( outline,
        [&]( const ci::vec2 &a, const ci::vec2 &b ) {
            ci::vec2 diff = b - a;
            *out++ = atan2( diff.y, diff.x );
        }
    );
}
std::vector<float> anglesBetweenPointsIn( const ci::PolyLine2f &outline );
std::vector<float> distanceBetweenPointsIn( const ci::PolyLine2f &outline );

// Determine the minimum area oriented bounding box for a polyline.
ci::Rectf minimumOobFor( const ci::PolyLine2f &outline, float &rotate );

// Determing how to divide an OOB in half perpendicular to its longest side.
// Fraction determines how much in on one side.
seg2 oobDivider( const ci::PolyLine2f &outline, float fraction = 0.5 );
// Given a bounding box rotated at an angle, return a segment to divide it long
// ways. Fraction controlls how much is on one side.
seg2 oobDivider( const ci::Rectf &bounds, float angle, float fraction = 0.5 );


// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<seg2> computeDividers( const std::vector<ci::vec2> &outline,
    const float angle = 0, const float width = 100 );

// Expand a line into rectangle with a thickness.
ci::PolyLine2f rectangleFrom( const ci::vec2 &start, const ci::vec2 &end, uint8_t width = 10 );

ci::Shape2d shapeFrom( const std::vector<ci::vec2> &points, bool closed = false );
ci::Shape2d shapeFrom( const ci::PolyLine2f &polyline );

// Create circle (or simpler shape if you use a low number of subdivisons)
ci::PolyLine2f polyLineCircle( float radius, u_int8_t subdivisions );
ci::PolyLine2f polyLineTriangle();
ci::PolyLine2f polyLineSquare();
ci::PolyLine2f polyLineRectangle( const uint16_t width, const uint16_t height );
ci::PolyLine2f polyLineLShape();
ci::PolyLine2f polyLinePlus();
ci::PolyLine2f polyLineTee();
