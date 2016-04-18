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
        polylineCircle( 1, mSubdivisions ),
        ci::geom::Cone().direction( ci::vec3( 0, 0, 1 ) ).subdivisionsAxis( 8 ).subdivisionsHeight( 2 )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float diameter, float height )
            : Scenery::Instance( plan, ci::vec3( at, 3), 0, ci::ColorA( 0.41f, 0.60f, 0.22f, 1.0f ) ),
                diameter( diameter ), height( height )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::scale( glm::translate( position ), ci::vec3( diameter, diameter, height ) );
        }

        float diameter;
        float height;
    };
    typedef std::shared_ptr<Instance>  InstanceRef;

  public:
    static ConeTreeRef create() { return ConeTreeRef( new ConeTree() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float diameter, float height )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, diameter, height ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};


class SphereTree;
typedef std::shared_ptr<SphereTree>  SphereTreeRef;

class SphereTree : public Scenery {
  protected:
    SphereTree() : Scenery(
        polylineCircle( 1, mSubdivisions ),
        ci::geom::Sphere().subdivisions( mSubdivisions )
    ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float diameter )
            : Scenery::Instance( plan, ci::vec3( at, diameter ), 0, ci::ColorA( 0.41f, 0.60f, 0.22f, 0.75f ) ),
              diameter( diameter )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::scale( glm::translate( position ), ci::vec3( diameter ) );
        }

        float diameter;
    };

  public:
    static SphereTreeRef create() { return SphereTreeRef( new SphereTree() ); };

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float diameter )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, diameter ) );
    }

  protected:
    const u_int8_t mSubdivisions = 12;
};