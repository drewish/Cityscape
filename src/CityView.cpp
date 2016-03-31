//
//  CityView.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/2/16.
//
//

#include "CityView.h"
#include "FlatShape.h"
#include "BuildingPlan.h"
#include "Resources.h"

using namespace ci;


CityView::CityView( const Cityscape::CityModel &model )
{
    gl::GlslProgRef colorShader = gl::getStockShader( gl::ShaderDef().color() );

    gl::GlslProgRef buildingShader = ci::gl::GlslProg::create(
       app::loadResource( RES_BUILDING_VERT ),
       app::loadResource( RES_BUILDING_FRAG )
    );
    float hue = 0.1;
    buildingShader->uniform( "darkColor",   Color( CM_HSV, hue, 0.60, 0.25 ) );
    buildingShader->uniform( "mediumColor", Color( CM_HSV, hue, 0.55, 0.66 ) );
    buildingShader->uniform( "lightColor",  Color( CM_HSV, hue, 0.15, 1.00 ) );

    gl::GlslProgRef treeShader = ci::gl::GlslProg::create(
       app::loadResource( RES_TREE_VERT ),
       app::loadResource( RES_TREE_FRAG )
    );

    geom::Plane plane = geom::Plane()
        .size( model.dimensions.getSize() )
        .origin( vec3( 0, 0, -0.10 ) )
        .axes( vec3( 1, 0, 0 ), vec3( 0, -1, 0 ) );
    ground = gl::Batch::create( plane >> geom::Constant( geom::Attrib::COLOR, model.groundColor ), colorShader );

    for ( const auto &shape : model.pavement ) {
        auto mesh = shape->mesh() >> geom::Constant( geom::Attrib::COLOR, model.roadColor );
        roads.push_back( gl::Batch::create( mesh, colorShader ) );
    }

    std::map<Cityscape::TreeFamily, std::vector<InstanceData>> treeData;
    std::map<BuildingPlanRef, std::vector<InstanceData>> buildingData;

    for ( const auto &district : model.districts ) {
        auto mesh = district->shape->mesh()
            >> geom::Constant( geom::Attrib::COLOR, district->color )
            >> geom::Translate( vec3( 0, 0, -0.03 ) );
        districts.push_back( gl::Batch::create( mesh, colorShader ) );

        for ( const auto &block : district->blocks ) {
            auto mesh = block->shape->mesh()
                >> geom::Constant( geom::Attrib::COLOR, block->color )
                >> geom::Translate( vec3( 0, 0, -0.02 ) );
            blocks.push_back( gl::Batch::create( mesh, colorShader ) );

            for ( const auto &lot : block->lots ) {
                auto mesh = lot->shape->mesh()
                    >> geom::Constant( geom::Attrib::COLOR, lot->color )
                    >> geom::Translate( vec3( 0, 0, -0.01 ) );
                lots.push_back( gl::Batch::create( mesh, colorShader ) );

                for ( const auto &tree : lot->trees ) {
                    mat4 modelView = glm::scale( glm::translate( tree->position ), vec3( tree->diameter ) );
                    treeData[ tree->family ].push_back( InstanceData( modelView, tree->color ) );
                }

                if ( lot->building && lot->building->plan ) {
                    mat4 modelView = glm::translate( vec3( lot->building->position, 0 ) );
                    modelView = glm::rotate( modelView, lot->building->rotation, vec3( 0, 0, 1 ) );
                    buildingData[ lot->building->plan ].push_back( InstanceData( modelView, ColorA::white() ) );
                }
            }
        }
    }

    for ( auto &pair : treeData ) {
        auto type = pair.first;
        auto instances = pair.second;
        ci::geom::SourceMods geom;
        if ( type == Cityscape::TreeFamily::CONIFER_TREE ) {
            geom = geom::Cone().subdivisionsAxis( 10 ).height( 3.0 ).direction( vec3( 0, 0, 1 ) );
        } else {
            geom = geom::Sphere().subdivisions( 10 );
        }
        trees.push_back( InstanceBatch( buildBatch( treeShader, geom, instances ), instances.size() ) );
    }

    for ( auto &pair : buildingData ) {
        auto plan = pair.first;
        auto instances = pair.second;
        buildings.push_back( InstanceBatch( buildBatch( buildingShader, plan->geometry(), instances ), instances.size() ) );
    }
}

gl::BatchRef CityView::buildBatch( const gl::GlslProgRef &shader, const geom::SourceMods &geometry, const std::vector<InstanceData> &instances ) const
{
    size_t stride = sizeof( InstanceData );

    // create the VBO which will contain per-instance (rather than per-vertex) data
    gl::VboRef vbo = gl::Vbo::create( GL_ARRAY_BUFFER, instances.size() * stride, instances.data(), GL_STATIC_DRAW );

    // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
    geom::BufferLayout layout;
    // Doing all this the long way so it's easier to debug later.
    uint8_t matrixDim = sizeof( ci::mat4 ) / sizeof( float );
    uint8_t colorDim = sizeof( ci::vec4 ) / sizeof( float );
    layout.append( geom::Attrib::CUSTOM_0, matrixDim, stride, 0, 1 );
    layout.append( geom::Attrib::COLOR, colorDim, stride, sizeof( ci::mat4 ), 1 );

    gl::VboMeshRef mesh = gl::VboMesh::create( geometry );
    mesh->appendVbo( layout, vbo );

    return gl::Batch::create( mesh, shader, { { geom::Attrib::CUSTOM_0, "vInstanceModelMatrix" }, { geom::Attrib::COLOR, "vInstanceColor" } } );
}

void CityView::draw( const Options &options ) const
{
    gl::ScopedBlendAlpha scopedAlpha;

    ground->draw();

    if ( options.drawRoads ) {
        for ( const auto &batch : roads ) batch->draw();
    }
    if ( options.drawDistricts ) {
        for ( const auto &batch : districts ) batch->draw();
    }
    if ( options.drawBlocks ) {
        for ( const auto &batch : blocks ) batch->draw();
    }
    if ( options.drawLots ) {
        for ( const auto &batch : lots ) batch->draw();
    }
    if ( options.drawTrees ) {
        gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
        for ( const auto &treebits : trees ) {
            treebits.first->drawInstanced( treebits.second );
        }
    }
    if ( options.drawBuildings ) {
        gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
        for ( const auto &buildingbits : buildings ) {
            buildingbits.first->drawInstanced( buildingbits.second );
        }
    }
}