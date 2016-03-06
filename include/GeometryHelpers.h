#pragma once

// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<ci::vec2> computeDividers( const std::vector<ci::vec2> &outline,
                                       const float angle = 0,
                                       const float width = 100 );

// Simple type conversion
// TODO this should move into the BuildingPlan class.
ci::Shape2d shapeFrom( const ci::PolyLine2f &polyline );