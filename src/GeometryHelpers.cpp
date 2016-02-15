#include "GeometryHelpers.h"

using namespace ci;

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<vec2> computeDividers( const std::vector<vec2> &outline, const float angle, const float width )
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
    vec2 topLeft(    inv * vec3( bounds.getUpperLeft(), 1 ) );
    vec2 bottomLeft( inv * vec3( bounds.getLowerLeft(), 1 ) );
    // (direction is perpendicular to the vector from TL to BL)
    vec2 direction(  inv * vec3( 1, 0, 0 ) );

    // ...then step across from left to right creating "vertical" dividers. Skip
    // over the first since it'll just fall on the edge.
    std::vector<vec2> result;
    for ( float distance = width; distance < bounds.getWidth(); distance += width ) {
        vec2 step = direction * distance;
        result.push_back( step + topLeft );
        result.push_back( step + bottomLeft );
    }
    return result;
}

Shape2d shapeFrom( const PolyLine2f &polyline )
{
    const std::vector<vec2> &points = polyline.getPoints();
    Shape2d result;

    auto it = points.begin();
    result.moveTo( *it );
    while ( ++it != points.end() ) {
        result.lineTo( *it );
    }

    // TODO: Not sure this is necessary...
    if ( polyline.isClosed() ) {
        result.close();
    }

    return result;
}