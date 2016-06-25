#include "GeometryHelpers.h"

#include <limits>
using std::numeric_limits;

using namespace ci;

// For a polyline with points a,b,c,d that is marked open:
//   a->b, b->c, c->d, d->a
// if it's closed:
//   a->b, b->c, c->d
void pointsInPairs( const PolyLine2f &outline, std::function<void(const vec2&, const vec2&)> process )
{
    if ( outline.size() < 2 ) return;

    const std::vector<vec2> &points = outline.getPoints();
    for ( auto prev = points.begin(), curr = prev + 1; curr != points.end(); ++curr ) {
        process( *prev, *curr );
        prev = curr;
    }
    if ( !outline.isClosed() ) {
        process( points.back(), points.front() );
    }
}

std::vector<float> anglesBetweenPointsIn( const PolyLine2f &outline )
{
    std::vector<float> angles;
    pointsInPairs( outline,
        [&angles]( const vec2 &a, const vec2 &b ) {
            vec2 diff = a - b;
            angles.push_back( atan2( diff.y, diff.x ) );
        }
    );
    return angles;
}

std::vector<float> distanceBetweenPointsIn( const PolyLine2f &outline )
{
    std::vector<float> lengths;
    pointsInPairs( outline,
        [&lengths](const vec2 &a, const vec2 &b) {
            lengths.push_back( glm::distance( a, b ) );
        }
    );
    return lengths;
}


// Determine the minimum area oriented bounding box for a set of points.
bool minimumOobFor( const PolyLine2f &outline, Rectf &bestBounds, float &bestAngle )
{
    if ( outline.size() < 2 ) return false;

    // Find the angle between each pair of points.
    std::vector<float> angles;
    pointsInPairs( outline,
        [&angles]( const vec2 &a, const vec2 &b ) {
            vec2 diff = a - b;
            // We're building boxes so we don't care about the quadrants
            angles.push_back( std::abs( atan2( diff.y, diff.x ) ) );
        }
    );

    // Remove duplicate angles
    std::sort( angles.begin(), angles.end() );
    angles.resize( std::distance( angles.begin(), std::unique( angles.begin(), angles.end() ) ) );

    // Rotate the shape to align each edge with the axis and see which angle
    // yields the smallet bounding box.
    float minArea = numeric_limits<float>::max();
    for ( float angle : angles ) {
        mat3 matrix = rotate( mat3(), angle );
        std::vector<vec2> rotatedOutline;
        for( const auto &point : outline.getPoints() ) {
            rotatedOutline.push_back( vec2( matrix * vec3( point, 1 ) ) );
        }

        Rectf bounds = Rectf( rotatedOutline );

        float newArea = bounds.calcArea();
        if ( newArea < minArea ) {
            minArea = newArea;

            bestAngle = angle;
            bestBounds = bounds;
        }
    }

    // Add a little padding to make sure we intersect both sides.
    bestBounds.inflate( vec2( 1, 1 ) );

    return true;
}

seg2 oobDivider( const ci::Rectf &bounds, float angle )
{
    mat3 inv = rotate( mat3(), -angle );

    if ( bounds.getWidth() > bounds.getHeight() ) {
        float midX = ( bounds.x1 + bounds.x2 ) / 2;
        return seg2(
            vec2( inv * vec3( midX, bounds.y1, 1 ) ),
            vec2( inv * vec3( midX, bounds.y2, 1 ) )
        );
    } else {
        float midY = ( bounds.y1 + bounds.y2 ) / 2;
        return seg2(
            vec2( inv * vec3( bounds.x1, midY, 1 ) ),
            vec2( inv * vec3( bounds.x2, midY, 1 ) )
        );
    }
}

seg2 oobDivider( const ci::PolyLine2f &outline )
{
    ci::Rectf bounds;
    float rotate;
    minimumOobFor( outline, bounds, rotate );
    return oobDivider( bounds, rotate );
}

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<seg2> computeDividers( const std::vector<vec2> &outline, const float angle, const float width )
{
    // Rotate the shape to the desired angle and find the bounding box...
    glm::mat3 matrix = rotate( glm::mat3(), angle );
    std::vector<vec2> rotatedOutline;
    for( const auto &point : outline ) {
        rotatedOutline.push_back( vec2( matrix * vec3( point, 1 ) ) );
    }
    Rectf bounds = Rectf( rotatedOutline );

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

Shape2d shapeFrom( const std::vector<vec2> &points, bool closed )
{
    Shape2d result;

    auto it = points.begin();
    result.moveTo( *it );
    while ( ++it != points.end() ) {
        result.lineTo( *it );
    }

    // TODO: Not sure this is necessary...
    if ( closed ) {
        result.close();
    }

    return result;
}

Shape2d shapeFrom( const PolyLine2f &polyline )
{
    return shapeFrom( polyline.getPoints(), polyline.isClosed() );
}

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
    result.setClosed();

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