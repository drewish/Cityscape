#include "GeometryHelpers.h"

using namespace ci;

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<seg2> computeDividers( const std::vector<vec2> &outline, const float angle, const float width )
{
    // Rotate the shape to the desired angle and find the bounding box...
    glm::mat3 matrix = rotate( glm::mat3(), angle );
    std::vector<vec2> rotated;
    for( const auto &point : outline ) {
        rotated.push_back( vec2( matrix * vec3( point, 1 ) ) );
    }
    Rectf bounds = Rectf( rotated );

    // ...now figure out where the left edge of that box would be in the
    // unrotated space...
    mat3 inv = inverse( matrix );
    // Put some Y-axis padding to ensure the lines overlap
    vec2 topLeft(    inv * vec3( bounds.getUpperLeft() - vec2( 0, 10 ), 1 ) );
    vec2 bottomLeft( inv * vec3( bounds.getLowerLeft() + vec2( 0, 10 ), 1 ) );
    // (direction is perpendicular to the vector from TL to BL)
    vec2 direction(  inv * vec3( 1, 0, 0 ) );

    // ...then step across from left to right creating "vertical" dividers. Skip
    // over the first since it'll just fall on the edge.
    std::vector<seg2> result;
    for ( float distance = width; distance < bounds.getWidth(); distance += width ) {
        vec2 step = direction * distance;
        result.push_back( seg2( topLeft + step, bottomLeft + step ) );
    }
    return result;
}

PolyLine2f rectangleFrom( const ci::vec2 &a, const ci::vec2 &b, uint8_t width )
{
    ci::vec2 perpendicular = glm::normalize( ci::vec2( b.y - a.y, -( b.x - a.x ) ) );
    ci::vec2 offset = perpendicular * ci::vec2( width / 2.0 );

    return PolyLine2f( { b + offset, b - offset, a - offset, a + offset } );
};

ci::PolyLine2f polyLineCircle( float radius, u_int8_t subdivisions )
{
    ci::PolyLine2f result;
    const ci::vec2 center( 0 );
    // iterate the segments
    const float tDelta = 1 / (float) subdivisions * 2.0f * M_PI;
    float t = 0;
    for( int s = 0; s <= subdivisions; s++ ) {
        ci::vec2 unit( ci::math<float>::cos( t ), ci::math<float>::sin( t ) );
        result.push_back( center + unit * radius );
        t += tDelta;
    }

    return result;
}

ci::PolyLine2f polyLineTriangle()
{
    ci::PolyLine2f result( {
        ci::vec2(10, -10), ci::vec2(10, 10), ci::vec2(-10, 10),
        ci::vec2(10, -10) // closure
    } );
    result.setClosed();
    return result;
}

ci::PolyLine2f polyLineSquare()
{
    return polyLineRectangle( 20, 20 );
}

ci::PolyLine2f polyLineRectangle( const uint16_t width, const uint16_t depth )
{
    float w = width / 2.0;
    float d = depth / 2.0;
    ci::PolyLine2f result( {
        ci::vec2(  w, -d ), ci::vec2(  w,  d ),
        ci::vec2( -w,  d ), ci::vec2( -w, -d ),
        ci::vec2(  w, -d ) // closure
    } );
    result.setClosed();
    return result;
}

ci::PolyLine2f polyLineLShape()
{
    ci::PolyLine2f result( {
        ci::vec2(15, 0), ci::vec2(15, 10), ci::vec2(-15, 10),
        ci::vec2(-15, -10), ci::vec2(-5, -10), ci::vec2(-5, 0),
        ci::vec2(15, 0) // closure
    } );
    result.setClosed();
    return result;
}

ci::PolyLine2f polyLinePlus()
{
    ci::PolyLine2f result( {
        ci::vec2(15,-5), ci::vec2(15,5), ci::vec2(5,5),
        ci::vec2(5,15), ci::vec2(-5,15), ci::vec2(-5,5),
        ci::vec2(-15,5), ci::vec2(-15,-5), ci::vec2(-5,-5),
        ci::vec2(-5,-15), ci::vec2(5,-15), ci::vec2(5,-5),
        ci::vec2(15,-5) // closure
    } );
    result.setClosed();
    return result;
}

ci::PolyLine2f polyLineTee()
{
    ci::PolyLine2f result( {
        ci::vec2(5,10), ci::vec2(-5,10), ci::vec2(-5,0),
        ci::vec2(-15,0), ci::vec2(-15,-10), ci::vec2(15,-10),
        ci::vec2(15,0), ci::vec2(5,0),
        ci::vec2(5,10) // closure
    } );
    result.setClosed();
    return result;
}