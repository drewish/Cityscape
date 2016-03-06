//
//  CityView.hpp
//  Cityscape
//
//  Created by Andrew Morton on 3/2/16.
//
//

#pragma once

#include "CityData.h"
#include "RoadNetwork.h"

class CityView;
typedef std::shared_ptr<CityView>   CityViewRef;

class CityView {
public:
    struct TreeInstance {
        TreeInstance( const ci::mat4 &mv ) : modelView( mv ) {};

        ci::mat4 modelView;
    };

    // Batch has mesh, instanced data, shader, size is number of instances to render.
    typedef std::pair<ci::gl::BatchRef, size_t> InstanceBatch;

    struct Options {
        bool drawRoads = true;
        bool drawDistricts = false;
        bool drawBlocks = false;
        bool drawLots = false;
        bool drawTrees = true;
        bool drawBuildings = true;
    };

    static CityViewRef create( const Cityscape::CityModel &cm ) { return CityViewRef( new CityView( cm ) ); }

    CityView( const Cityscape::CityModel &model );

    ci::gl::BatchRef treeBatch( const ci::gl::GlslProgRef &shader, const std::vector<TreeInstance> &trees ) const;
    ci::gl::BatchRef buildingBatch( const ci::gl::GlslProgRef &shader, const Building &building ) const;

    void draw( const Options &o ) const;

    std::vector<ci::gl::BatchRef> roads;
    std::vector<ci::gl::BatchRef> districts;
    std::vector<ci::gl::BatchRef> blocks;
    std::vector<ci::gl::BatchRef> lots;
    std::vector<InstanceBatch> buildings;
    std::vector<InstanceBatch> trees;
};
