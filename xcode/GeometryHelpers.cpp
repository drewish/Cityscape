#include "GeometryHelpers.h"

using namespace ci;

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<vec2> computeDividers( const std::vector<vec2> &outline, const float angle, const float width )
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