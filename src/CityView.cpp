//
//  CityView.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/2/16.
//
//

#include "CityView.h"
#include "FlatShape.h"
#include "Resources.h"

using namespace ci;


CityView::CityView(const Cityscape::CityModel &model)
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

    for ( const auto &shape : model.pavement ) {
        auto mesh = shape->mesh() >> geom::Constant( geom::Attrib::COLOR, model.options.road.color );
        roads.push_back( gl::Batch::create( mesh, colorShader ) );
    }

    std::vector<TreeInstance> treeData;

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
                    treeData.push_back( TreeInstance( modelView ) );
                }

//                if ( lot->mBuildingRef ) {
//                    buildings.push_back( InstanceBatch( buildingBatch( buildingShader, *lot->mBuildingRef ), 1 ) );
//                }
            }
        }
    }
    trees.push_back( InstanceBatch( treeBatch( treeShader, treeData ), treeData.size() ) );
}

gl::BatchRef CityView::treeBatch( const gl::GlslProgRef &shader, const std::vector<TreeInstance> &trees ) const
{
    size_t stride = sizeof( TreeInstance );

    // create the VBO which will contain per-instance (rather than per-vertex) data
    gl::VboRef instanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, trees.size() * stride, trees.data(), GL_STATIC_DRAW );

    // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
    geom::BufferLayout instanceDataLayout;
    instanceDataLayout.append( geom::Attrib::CUSTOM_0, stride / sizeof( float ), stride, 0, 1 );

    gl::VboMeshRef mesh = gl::VboMesh::create( geom::Sphere().subdivisions( 12 ) );
    mesh->appendVbo( instanceDataLayout, instanceDataVbo );

    return gl::Batch::create( mesh, shader, { { geom::Attrib::CUSTOM_0, "vInstanceModelMatrix" } } );
}

gl::BatchRef CityView::buildingBatch( const gl::GlslProgRef &shader, const Building &building ) const
{
    const BuildingPlan &plan = building.mPlan;

    mat4 buildingTransform = glm::translate( vec3( building.mPosition, 0 ) );
    buildingTransform = glm::rotate( buildingTransform, building.mRotation, vec3( 0, 0, 1 ) );

    // Scale the walls upwards for multiple floors
    mat4 wallTransform = glm::scale( buildingTransform, vec3( 1, 1, building.mFloors ) );
    geom::SourceMods walls = *plan.wallMeshRef()->createSource() >> geom::Transform( wallTransform );

    // Move the roof up to the top of the walls
    mat4 roofTransform = glm::translate( buildingTransform, vec3( 0, 0, building.mFloors * plan.floorHeight() ) );
    geom::SourceMods roof = *plan.roofMeshRef()->createSource() >> geom::Transform( roofTransform );

    // Merge the roof an walls into one batch
    return gl::Batch::create( roof & walls, shader );
}

void CityView::draw( const Options &options ) const
{
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
            // TODO move the color into the instance settings
            gl::ScopedColor scopedColor( ColorA8u( 0x69, 0x98, 0x38, 0xC0 ) );
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