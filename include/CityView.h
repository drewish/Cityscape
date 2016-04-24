//
//  CityView.hpp
//  Cityscape
//
//  Created by Andrew Morton on 3/2/16.
//
//

#pragma once

#include "CityData.h"

class CityView;
typedef std::shared_ptr<CityView>   CityViewRef;

class CityView {
public:
    struct InstanceData {
        InstanceData( const ci::mat4 &mv, const ci::vec4 &c )
            : modelView( mv ), color( c ) {};

        ci::mat4 modelView;
        ci::vec4 color;
    };

    // Batch has mesh, instanced data, shader, size is number of instances to render.
    typedef std::pair<ci::gl::BatchRef, size_t> InstanceBatch;

    struct Options {
        bool drawRoads = true;
        bool drawDistricts = false;
        bool drawBlocks = false;
        bool drawLots = false;
        bool drawPlants = true;
        bool drawBuildings = true;
    };

    static CityViewRef create( const Cityscape::CityModel &cm ) { return CityViewRef( new CityView( cm ) ); }

    CityView( const Cityscape::CityModel &model );

    ci::gl::BatchRef buildBatch( const ci::gl::GlslProgRef &shader, const ci::geom::SourceMods &geometry, const std::vector<InstanceData> &instances ) const;

    void draw( const Options &o ) const;

    ci::gl::BatchRef              ground;
    std::vector<ci::gl::BatchRef> roads;
    std::vector<ci::gl::BatchRef> districts;
    std::vector<ci::gl::BatchRef> blocks;
    std::vector<ci::gl::BatchRef> lots;
    std::vector<InstanceBatch> buildings;
    std::vector<InstanceBatch> plants;
};
