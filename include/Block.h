//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Block__
#define __Cityscape__Block__

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/create_straight_skeleton_2.h>

class Lot;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Straight_skeleton_2<K> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

class Block {
public:
    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    Block( const ci::PolyLine2f outline ) : outline(outline) { };

    void drawSkeleton(const SsPtr &ss);

    void draw();
    void subdivide();
    
    ci::PolyLine2f outline;
    std::vector<Lot> lots;
    SsPtr mSkel;
};

#endif /* defined(__Cityscape__Block__) */
