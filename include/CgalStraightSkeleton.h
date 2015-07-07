//
//  CgalStraightSkeleton.h
//  Cityscape
//
//  Created by Andrew Morton on 7/7/15.
//
//

#ifndef Cityscape_CgalStraightSkeleton_h
#define Cityscape_CgalStraightSkeleton_h

#include "CgalKernel.h"
#include <CGAL/create_straight_skeleton_2.h>

typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

#endif
