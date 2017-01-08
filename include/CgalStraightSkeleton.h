//
//  CgalStraightSkeleton.h
//  Cityscape
//
//  Created by Andrew Morton on 7/7/15.
//
//

#pragma once

#include "CgalKernel.h"
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/create_offset_polygons_2.h>

typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;
typedef boost::shared_ptr<CGAL::Polygon_2<InexactK>> PolygonPtr;

inline PolygonPtr expandPolygon( double offset, const CGAL::Polygon_2<InexactK> &input ) {
    // First outline is the bounding box, remaining ones are the shape and its
    // holes. We're assuming no holes.
    std::vector<PolygonPtr> outlines = CGAL::create_exterior_skeleton_and_offset_polygons_2( offset, input,                                                                                                      InexactK(), InexactK() );
    outlines.back()->reverse_orientation();
    return outlines.back();
}
