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
    ConeTree() : Scenery(
        polyLineCircle( 0.5, mSubdivisions ),
        ci::geom::Cone().radius( 0.5, 0.0 ).height( 1 ).direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( 8 ).subdivisionsHeight( 2 )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float radius, float height )
            : Scenery::Instance( plan, ci::vec3( at, 3 ), 0, ci::ColorA( 0.41f, 0.60f, 0.22f, 1.0f ) ),
                radius( radius ), height( height )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::scale( glm::translate( position ), ci::vec3( radius, radius, height ) );
        }

        float radius;
        float height;
    };
    typedef std::shared_ptr<Instance>  InstanceRef;

  public:
    static ConeTreeRef create() { return ConeTreeRef( new ConeTree() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float radius, float height )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, radius, height ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};


class SphereTree;
typedef std::shared_ptr<SphereTree>  SphereTreeRef;

class SphereTree : public Scenery {
  protected:
    SphereTree() : Scenery(
        polyLineCircle( 0.5, mSubdivisions ),
        ci::geom::Sphere().radius( 0.5 ).subdivisions( mSubdivisions )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float radius )
            : Scenery::Instance( plan, ci::vec3( at, radius ), 0, ci::ColorA( 0.41f, 0.60f, 0.22f, 0.75f ) ),
              radius( radius )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::scale( glm::translate( position ), ci::vec3( radius ) );
        }

        float radius;
    };

  public:
    static SphereTreeRef create() { return SphereTreeRef( new SphereTree() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float radius )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, radius ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};


class RowCrop;
typedef std::shared_ptr<RowCrop>  RowCropRef;

class RowCrop : public Scenery {
  protected:
    RowCrop() : Scenery(
        polyLineRectangle( 1, 1 ),
        ci::geom::Extrude( shapeFrom( { ci::vec2( 0.5, 0 ), ci::vec2( 0, 1 ), ci::vec2( -0.5, 0 ) } ), 1.0f, 1.0f ) >> ci::geom::Rotate( M_PI_2, ci::vec3( 1, 0, 0 ) )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float rotation, float length, float width )
            : Scenery::Instance( plan, ci::vec3( at, 0 ), rotation, ci::ColorA( 0.41f, 0.60f, 0.22f, 0.75f ) ),
              length( length ), width( width )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            ci::mat4 result;
            result = glm::translate( result, position );
            // TODO: Be nice to figure out why I have to either have a -Z axis
            // or inverted rotation.
            result = glm::rotate( result, rotation, ci::vec3( 0, 0, -1 ) );
            result = glm::scale( result, ci::vec3( width, length, 1 ) );
            return result;
        }

        float length;
        float width;
    };

  public:
    static RowCropRef create() { return RowCropRef( new RowCrop() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &start, const ci::vec2 &end, float width )
    {
        ci::vec2 midpoint( ( start + end ) / ci::vec2( 2.0 ) );
        float rotation = atan2( end.x - start.x, end.y - start.y );
        float length = glm::distance( start, end );
        return Scenery::InstanceRef( new Instance( shared_from_this(), midpoint, rotation, length, width ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};

// * * *

class SmokeStack;
typedef std::shared_ptr<SmokeStack>  SmokeStackRef;

class SmokeStack : public Scenery {
  protected:
    SmokeStack() : Scenery(
        polyLineCircle( 0.5, mSubdivisions ),
        ci::geom::Cone().radius( 0.5, 0.4 ).height( 1 ).direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( 8 ).subdivisionsHeight( 2 )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float radius, float height )
            : Scenery::Instance( plan, ci::vec3( at, radius ), 0, ci::Color::white() ),
              radius( radius ), height( height )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::scale( glm::translate( position ), ci::vec3( radius, radius, height ) );
        }

        float radius;
        float height;
    };

  public:
    static SmokeStackRef create() { return SmokeStackRef( new SmokeStack() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float radius = 2.0, float height = 20.0 )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, radius, height ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};



class OilTank;
typedef std::shared_ptr<OilTank>  OilTankRef;

class OilTank : public Scenery {
  protected:
    OilTank() : Scenery(
        polyLineCircle( 0.5, mSubdivisions ),
        ci::geom::Cylinder().radius( 0.5 ).height( 1 ).subdivisionsAxis( mSubdivisions ).direction( ci::vec3( 0, 0, 1 ) )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float radius, float height )
            : Scenery::Instance( plan, ci::vec3( at, radius ), 0, ci::Color::white() ),
              radius( radius ), height( height )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::scale( glm::translate( position ), ci::vec3( radius, radius, height ) );
        }

        float radius;
        float height;
    };

  public:
    static OilTankRef create() { return OilTankRef( new OilTank() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float radius = 20.0, float height = 10.0 )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, radius, height ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};