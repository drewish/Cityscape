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

ci::gl::BatchRef buildSky()
{
    std::vector<vec3> positions;
    std::vector<Color> colors;
    Color darkBlue = Color8u(108, 184, 251);
    Color medBlue = Color8u(160, 212, 250);
    Color lightBlue = Color8u(174, 214, 246);

    positions.push_back( vec3( +0.5, -0.5, +0.0 ) );
    positions.push_back( vec3( -0.5, -0.5, +0.0 ) );
    colors.push_back( darkBlue );
    colors.push_back( darkBlue );
    positions.push_back( vec3( +0.5, -0.2, +0.0 ) );
    positions.push_back( vec3( -0.5, -0.2, +0.0 ) );
    colors.push_back( medBlue );
    colors.push_back( medBlue );
    positions.push_back( vec3( +0.5, +0.2, +0.0 ) );
    positions.push_back( vec3( -0.5, +0.2, +0.0 ) );
    colors.push_back( medBlue );
    colors.push_back( medBlue );
    positions.push_back( vec3( +0.5, +0.5, +0.0 ) );
    positions.push_back( vec3( -0.5, +0.5, +0.0 ) );
    colors.push_back( lightBlue );
    colors.push_back( lightBlue );

    std::vector<gl::VboMesh::Layout> bufferLayout = {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::COLOR, 3 ),
    };
    gl::VboMeshRef mesh = gl::VboMesh::create( positions.size(), GL_TRIANGLE_STRIP, bufferLayout );
    mesh->bufferAttrib( geom::Attrib::POSITION, positions );
    mesh->bufferAttrib( geom::Attrib::COLOR, colors );

    gl::GlslProgRef colorShader = gl::getStockShader( gl::ShaderDef().color() );

    return gl::Batch::create( mesh, colorShader );
}

ci::gl::BatchRef buildGround( const Cityscape::CityModel &model )
{
    geom::Plane plane = geom::Plane()
        .size( model.dimensions.getSize() )
        .origin( vec3( 0, 0, -0.10 ) )
        .axes( vec3( 1, 0, 0 ), vec3( 0, -1, 0 ) );

    gl::GlslProgRef colorShader = gl::getStockShader( gl::ShaderDef().color() );

    return gl::Batch::create( plane >> geom::Constant( geom::Attrib::COLOR, model.groundColor ), colorShader );
}

gl::BatchRef buildBatch( const gl::GlslProgRef &shader, const geom::SourceMods &geometry, const std::vector<CityView::InstanceData> &instances )
{
    size_t stride = sizeof( CityView::InstanceData );

    // create the VBO which will contain per-instance (rather than per-vertex) data
    gl::VboRef vbo = gl::Vbo::create( GL_ARRAY_BUFFER, instances.size() * stride, instances.data(), GL_STATIC_DRAW );

    // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic,
    // and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
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


typedef std::map<SceneryRef, std::vector<CityView::InstanceData>> GroupedScenery;

void collectInstanceData( GroupedScenery &data, const Scenery::Instance &instance )
{
    if( instance.scenery->children().size() ) {
        for( auto child : instance.scenery->children() ) {
            collectInstanceData( data, child );
        }
    } else {
        data[ instance.scenery ].push_back(
            CityView::InstanceData(
                instance.transformation,
                instance.color
            )
        );
    }
}

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

    sky = buildSky();
    ground = buildGround( model );

    for ( const auto &shape : model.pavement ) {
        auto mesh = *shape->mesh() >> geom::Constant( geom::Attrib::COLOR, model.roadColor );
        roads.push_back( gl::Batch::create( mesh, colorShader ) );
    }

    GroupedScenery buildingData;
    GroupedScenery plantData;

    for ( const auto &district : model.districts ) {
        auto mesh = *district->shape->mesh()
            >> geom::Constant( geom::Attrib::COLOR, district->color )
            >> geom::Translate( vec3( 0, 0, -0.03 ) );
        districts.push_back( gl::Batch::create( mesh, colorShader ) );

        for ( const auto &block : district->blocks ) {
            auto mesh = *block->shape->mesh()
                >> geom::Constant( geom::Attrib::COLOR, block->color )
                >> geom::Translate( vec3( 0, 0, -0.02 ) );
            blocks.push_back( gl::Batch::create( mesh, colorShader ) );

            for ( const auto &lot : block->lots ) {
                auto mesh = *lot->shape->mesh()
                    >> geom::Constant( geom::Attrib::COLOR, lot->color )
                    >> geom::Translate( vec3( 0, 0, -0.01 ) );
                lots.push_back( gl::Batch::create( mesh, colorShader ) );

                for ( const auto &instance : lot->buildings ) {
                    collectInstanceData( buildingData, instance );
                }

                for ( const auto &instance : lot->plants ) {
                    collectInstanceData( plantData, instance );
                }

                for ( seg2 side : lot->streetFacingSides ) {
                    lotEdges.push_back( PolyLine2f( { side.first, side.second } ) );
                }
            }
        }
    }

    for ( auto &pair : plantData ) {
        auto plan = pair.first;
        auto instances = pair.second;
        plants.push_back( InstanceBatch( buildBatch( treeShader, plan->geometry(), instances ), instances.size() ) );
    }

    for ( auto &pair : buildingData ) {
        auto plan = pair.first;
        auto instances = pair.second;
        buildings.push_back( InstanceBatch( buildBatch( buildingShader, plan->geometry(), instances ), instances.size() ) );
    }
}

void CityView::draw( const Options &options ) const
{
    {
        // Fill the screen with our sky... at some point it should probably
        // become a skybox since the gradient doesn't move with the camera.
        gl::ScopedMatrices matrixScope;
        vec2 window = ci::app::getWindowSize();
        gl::setMatricesWindow( window );
        gl::translate( window.x / 2.0, 0.5 * window.y );
        gl::scale( window.x, window.y, 1 );

        sky->draw();
    }

    gl::ScopedDepth depthScope(true);
    gl::ScopedBlendAlpha scopedAlpha;

    ground->draw();

    // Transparent ground layers are ordered from the ground up so we're ignoring
    // further depth sorting.
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
    if ( options.drawLotEdges ) {
        gl::ScopedColor color( Color::black() );
        for ( const auto &edge : lotEdges ) {
            gl::draw( edge );
        }
    }

    // Draw opaque objects. TODO: sort front-to-back to aid z-culling.
    if ( options.drawBuildings ) {
        gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
        for ( const auto &buildingbits : buildings ) {
            buildingbits.first->drawInstanced( buildingbits.second );
        }
    }
    // Draw transparent objects. TODO: should sort them back to front
    if ( options.drawPlants ) {
        gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
        for ( const auto &plant : plants ) {
            plant.first->drawInstanced( plant.second );
        }
    }
}
