//
//  CityView.hpp
//  Cityscape
//
//  Created by Andrew Morton on 3/2/16.
//
//

#pragma once

#include "RoadNetwork.h"
//TODO: need to extract the draw options into a separate struct
#include "Options.h"

namespace CityScape {

    class CityView;
    typedef std::shared_ptr<CityView>   CityViewRef;

    // Batch has mesh, instanced data, shader, size is number of instances to render.
    typedef std::pair<ci::gl::BatchRef, size_t> InstanceBatch;

    class CityView {
    public:

        static CityViewRef create( const RoadNetwork &r ) { return CityViewRef( new CityView( r ) ); }

        CityView( const RoadNetwork &model );

        ci::gl::BatchRef treeBatch( const std::vector<Tree> &trees ) const;

        void draw( const Options &o ) const;

        std::vector<ci::gl::BatchRef> roads;
        std::vector<ci::gl::BatchRef> blocks;
        std::vector<ci::gl::BatchRef> lots;
        std::vector<InstanceBatch> buildings;
        std::vector<InstanceBatch> trees;
    };
}
