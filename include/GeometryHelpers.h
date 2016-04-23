#pragma once

typedef std::pair<ci::vec2, ci::vec2> seg2;

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<seg2> computeDividers( const std::vector<ci::vec2> &outline,
    const float angle = 0, const float width = 100 );

// Expand a line into rectangle with a thickness.
ci::PolyLine2f rectangleFrom( const ci::vec2 &start, const ci::vec2 &end, uint8_t width = 10 );


// Create circle (or simpler shape if you use a low number of subdivisons)
ci::PolyLine2f polyLineCircle( float radius, u_int8_t subdivisions );
ci::PolyLine2f polyLineTriangle();
ci::PolyLine2f polyLineSquare();
ci::PolyLine2f polyLineRectangle( const uint16_t width, const uint16_t height );
ci::PolyLine2f polyLineLShape();
ci::PolyLine2f polyLinePlus();
ci::PolyLine2f polyLineTee();