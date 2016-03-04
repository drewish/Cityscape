//
//  RoadNetwork.h
//  Cityscape
//
//  Created by Andrew Morton on 5/31/15.
//
//

#pragma once

#include "Options.h"
#include "Block.h"
#include "FlatShape.h"

#include <CGAL/Polygon_set_2.h>

class RoadNetwork {
public:
    RoadNetwork() {}

    void clear()
    {
        mPoints.clear();
    }

    void addPoints( const std::vector<ci::vec2> &points )
    {
        mPoints.insert( mPoints.end(), points.begin(), points.end() );
    }

    void addPoint( const ci::vec2 &pos )
    {
        mPoints.push_back( pos );
    }

    const std::vector<ci::vec2> getPoints() const { return mPoints; }

    void layout( const Options &options );

    // TODO: move to CityModel
    std::vector<Block> mBlocks;
    std::vector<FlatShape> mRoadShapes;
    ci::ColorA mRoadColor = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f );

private:
    void buildHighways( const Options &options, CGAL::Polygon_set_2<ExactK> &paved );
    void buildSideStreets( const Options &options, CGAL::Polygon_set_2<ExactK> &paved );
    void buildBlocks( const Options &options );

    CGAL::Polygon_2<ExactK> roadOutline( const ci::vec2 &a, const ci::vec2 &b, uint8_t width = 10 );

    std::vector<ci::vec2> mPoints;

};
