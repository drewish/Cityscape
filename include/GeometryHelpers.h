#pragma once

typedef std::pair<ci::vec2, ci::vec2> seg2;

// For a polyline with points a,b,c,d that is marked open:
//   a->b, b->c, c->d, d->a
// if it's closed:
//   a->b, b->c, c->d
void pointsInPairs( const ci::PolyLine2f &outline, std::function<void(const ci::vec2&, const ci::vec2&)> process );

// Find the angle in radians between each pair of points.
std::vector<float> anglesBetweenPointsIn( const ci::PolyLine2f &outline );
std::vector<float> distanceBetweenPointsIn( const ci::PolyLine2f &outline );

// Determine the minimum area oriented bounding box for a polyline.
bool minimumOobFor( const ci::PolyLine2f &outline, ci::Rectf &bounds, float &rotate );

// Determing how to divide an OOB in half.
seg2 oobDivider( const ci::PolyLine2f &outline );
seg2 oobDivider( const ci::Rectf &bounds, float angle );

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