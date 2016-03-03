//
//  CityView.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/2/16.
//
//

#include "CityView.h"

using namespace ci;

namespace CityScape {
    CityView::CityView(const RoadNetwork &model)
    {
        gl::GlslProgRef colorShader = gl::getStockShader( gl::ShaderDef().color() );

        for ( const auto &shape : model.mRoadShapes ) {
            auto mesh = shape.mesh() >> geom::Constant( geom::Attrib::COLOR, model.mRoadColor );
            roads.push_back( gl::Batch::create( mesh, colorShader ) );
        }

        std::vector<Tree> treeData;

        for ( const auto &block : model.mBlocks ) {
            auto mesh = block.mShape.mesh() >> geom::Constant( geom::Attrib::COLOR, block.mColor );
            blocks.push_back( gl::Batch::create( mesh, colorShader ) );

            for ( const auto &lot : block.mLots ) {
                auto mesh = lot->mShape.mesh() >> geom::Constant( geom::Attrib::COLOR, lot->mColor );
                lots.push_back( gl::Batch::create( mesh, colorShader ) );

                for ( const auto &tree : lot->mTrees ) {
                    treeData.push_back( tree );
                }
            }
        }

        trees.push_back( InstanceBatch( treeBatch( treeData ), treeData.size() ) );
    }

    gl::BatchRef CityView::treeBatch( const std::vector<Tree> &trees ) const
    {
        gl::GlslProgRef shader = gl::GlslProg::create( gl::GlslProg::Format()
            .vertex(
                CI_GLSL( 150,
                    uniform mat4	ciModelViewProjection;
                    uniform mat3	ciNormalMatrix;

                    in vec4		ciPosition;
                    in vec4		ciColor;
                    in vec3		vInstancePosition;  // per-instance position variable
                    in float    vInstanceScale;     // per-instance position variable

                    out lowp vec4	Color;

                    void main( void )
                    {
                        // TODO: need to figure out how to scale the tree up based on the vInstanceScale param
                        gl_Position	= ciModelViewProjection * ( ciPosition + vec4( vInstancePosition, 0 ) );
                        Color 		= ciColor;
                    }
                )
            )
            .fragment(
                     CI_GLSL( 150,
                        in vec4 Color;

                        out vec4 oColor;

                        void main( void )
                        {
                            oColor = Color;
                        }
                    )
                )
            );

        gl::VboMeshRef mesh = gl::VboMesh::create( geom::Sphere().radius( 10 ).subdivisions( 12 ) );
        size_t recSize = sizeof( Tree );

        // create the VBO which will contain per-instance (rather than per-vertex) data
        gl::VboRef treeInstanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, trees.size() * recSize, trees.data(), GL_STATIC_DRAW );

        // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
        geom::BufferLayout instanceDataLayout;
        instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, recSize, 0, 1 );
        instanceDataLayout.append( geom::Attrib::CUSTOM_1, 1, recSize, sizeof( vec3 ), 1 );

        // now add it to the VboMesh we already made of the Teapot
        mesh->appendVbo( instanceDataLayout, treeInstanceDataVbo );

        return gl::Batch::create( mesh, shader, { { geom::Attrib::CUSTOM_0, "vInstancePosition" }, { geom::Attrib::CUSTOM_1, "vInstanceScale" } } );
    }

    void CityView::draw( const Options &options ) const
    {
        if ( options.drawRoads ) {
            for ( const auto &batch : roads )   batch->draw();
        }
        if ( options.drawBlocks ) {
            for ( const auto &batch : blocks )  batch->draw();
        }
        if ( options.drawLots ) {
            for ( const auto &batch : lots )    batch->draw();
        }
        for ( const auto &treebits : trees ) {
            gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
            gl::ScopedColor scopedColor( ColorA8u( 0x69, 0x98, 0x38, 0xC0 ) );
            treebits.first->drawInstanced( treebits.second );
        }
    }

}