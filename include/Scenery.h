//
//  Scenery.h
//  Cityscape
//
//  Created by Andrew Morton on 3/1/16.
//
//
#pragma once

#include "GeometryHelpers.h"
#include "CityData.h"


class ConeTree;
typedef std::shared_ptr<ConeTree>  ConeTreeRef;

class ConeTree : public Scenery {
  protected:
    ConeTree( u_int8_t subdivisions = 12 )
    :   Scenery(
            polyLineCircle( 0.5, subdivisions ),
            ci::geom::Cone().radius( 0.5, 0.0 ).height( 1 ).direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( subdivisions ).subdivisionsHeight( 2 )
        )
    {}

  public:
    const ci::ColorA DEFAULT_COLOR = ci::ColorA( 0.41f, 0.60f, 0.22f, 1.0f );

    static ConeTreeRef create() { return ConeTreeRef( new ConeTree() ); };

    static ci::mat4 buildMatrix( const ci::vec2 &position, float radius, float height )
    {
        return glm::scale( glm::translate( ci::vec3( position, 3 ) ), ci::vec3( radius, radius, height ) );
    }

    Scenery::Instance instance( const ci::vec2 &position, float radius, float height )
    {
        return Scenery::Instance( shared_from_this(), buildMatrix( position, radius, height ), DEFAULT_COLOR );
    }
};


class SphereTree;
typedef std::shared_ptr<SphereTree>  SphereTreeRef;

class SphereTree : public Scenery {
  protected:
    SphereTree( u_int8_t subdivisions = 12 )
    :   Scenery(
            polyLineCircle( 0.5, subdivisions ),
            ci::geom::Sphere().radius( 0.5 ).subdivisions( subdivisions )
        )
    {}

  public:
    const ci::ColorA DEFAULT_COLOR = ci::ColorA( 0.41f, 0.60f, 0.22f, 0.75f );

    static SphereTreeRef create() { return SphereTreeRef( new SphereTree() ); };

    static ci::mat4 buildMatrix( const ci::vec2 &position, float radius )
    {
       return glm::scale( glm::translate( ci::vec3( position, radius ) ), ci::vec3( radius ) );
    }

    Scenery::Instance instance( const ci::vec2 &at, float radius )
    {
        return Scenery::Instance( shared_from_this(), buildMatrix( at, radius ), DEFAULT_COLOR );
    }
};


class RowCrop;
typedef std::shared_ptr<RowCrop>  RowCropRef;

class RowCrop : public Scenery {
  protected:
    RowCrop() : Scenery(
        polyLineRectangle( 1, 1 ),
        ci::geom::Extrude( shapeFrom( { ci::vec2( 0.5, 0 ), ci::vec2( 0, 1 ), ci::vec2( -0.5, 0 ) } ), 1.0f, 1.0f )
            >> ci::geom::Rotate( M_PI_2, ci::vec3( 1, 0, 0 ) )
    ) {}

  public:
    const ci::ColorA DEFAULT_COLOR = ci::Color( 0.663, 0.502, 0.282 );

    static RowCropRef create() { return RowCropRef( new RowCrop() ); };

    static ci::mat4 buildMatrix( const ci::vec2 &start, const ci::vec2 &end, float width )
    {
        ci::vec2 midpoint( ( start + end ) / ci::vec2( 2.0 ) );
        float rotation = atan2( end.x - start.x, end.y - start.y );
        float length = glm::distance( start, end );

        ci::mat4 result;
        result = glm::translate( result, ci::vec3( midpoint, 0 ) );
        // TODO: Be nice to figure out why I have to either have a -Z axis
        // or inverted rotation.
        result = glm::rotate( result, rotation, ci::vec3( 0, 0, -1 ) );
        result = glm::scale( result, ci::vec3( width, length, 1 ) );
        return result;
    }

    Scenery::Instance instance( const ci::vec2 &start, const ci::vec2 &end, float width )
    {
        return Scenery::Instance( shared_from_this(), buildMatrix( start, end, width ), DEFAULT_COLOR );
    }
};

// * * *

class GrainSiloConeTop : public Scenery {
  public:
    GrainSiloConeTop( float radius, float height, float overhang, u_int8_t subdivisions = 12 )
    :   Scenery(
            polyLineCircle( radius, subdivisions ),
            ci::geom::SourceMods()
                & ci::geom::Cone().base( radius + overhang ).height( radius / 2.0 ).direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( subdivisions ) >> ci::geom::Translate( ci::vec3( 0, 0, height ) )
                & ci::geom::Cylinder().radius( radius ).height( height ).direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( subdivisions )

        )
    {}

    static SceneryRef create( float radius = 2.5, float height = 10.0, float overhang = 0.25 )
    {
        return SceneryRef( new GrainSiloConeTop( radius, height, overhang ) );
    };
};

// * * *

class SmokeStack : public Scenery {
  public:
    SmokeStack( float radius, float height, u_int8_t subdivisions = 12 )
    :   Scenery(
            polyLineCircle( radius * 0.5, subdivisions ),
            ci::geom::Cone().base( radius * 0.5 ).apex( radius * 0.4 ).height( height )
                .direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( subdivisions )
                .subdivisionsHeight( 2 )
        )
    {}

    static SceneryRef create( float radius = 2.0, float height = 20.0 )
    {
        return SceneryRef( new SmokeStack( radius, height ) );
    };
};

class OilTank : public Scenery {
  public:
    OilTank( float radius, float height, u_int8_t subdivisions = 12 )
    :   Scenery(
            polyLineCircle( radius * 0.5, subdivisions ),
            ci::geom::Cylinder().radius( radius * 0.5 ).height( height )
                .subdivisionsAxis( subdivisions ).direction( ci::vec3( 0, 0, 1 ) )
        )
    {}

    static SceneryRef create( float radius = 20.0, float height = 10.0 )
    {
        return SceneryRef( new OilTank( radius, height ) );
    };
};
