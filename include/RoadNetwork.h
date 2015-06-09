//
//  RoadNetwork.h
//  Cityscape
//
//  Created by Andrew Morton on 5/31/15.
//
//

#ifndef __Cityscape__RoadNetwork__
#define __Cityscape__RoadNetwork__

#include "Options.h"
#include "Road.h"
#include "Block.h"
#include "FlatShape.h"

#include <CGAL/Polygon_set_2.h>

class RoadNetwork {
public:
    RoadNetwork() {}

    void clear()
    {
        mPoints.clear();
        layout();
    }

    // Insert multiple points and avoid a layout for each.
    void addPoints( const std::vector<ci::Vec2f> &points )
    {
        mPoints.insert( mPoints.end(), points.begin(), points.end() );
        layout();
    }

    void addPoint( const ci::Vec2f &pos )
    {
        mPoints.push_back( pos );
        // Don't bother redoing the layout if we have an odd number of points.
        if ( mPoints.size() % 2 == 0 ) {
            layout();
        }
    }
    void layout();
    void draw( const Options &options );

private:
    void buildHighways( CGAL::Polygon_set_2<ExactK> &paved );
    void buildSideStreets( CGAL::Polygon_set_2<ExactK> &paved );
    void buildBlocks();

    std::vector<ci::Vec2f> mPoints;
    std::vector<Block> mBlocks;
    std::vector<FlatShape> mShapes;
};

#endif /* defined(__Cityscape__RoadNetwork__) */
