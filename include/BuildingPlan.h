//
//  BuildingPlan.h
//  Cityscape
//
//  Created by andrew morton on 6/8/15.
//
//

#pragma once

#include "GeometryHelpers.h"

class Blueprint;
typedef std::shared_ptr<Blueprint>  BlueprintRef;

class Blueprint {
  public:
    Blueprint( const ci::PolyLine2f &outline ) : mOutline( outline ) {};

    // 2d footprint of the structure. The optional params can be used to
    // tranform the outline to reflect the position in the lot.
    ci::PolyLine2f outline( const ci::vec2 offset = ci::vec2( 0 ), const float rotation = 0.0 ) const;
    // 3d view of the structure
    const ci::geom::SourceMods &geometry() const { return mGeometry; };

  protected:
    ci::PolyLine2f          mOutline;
    ci::geom::SourceMods    mGeometry;
};

class BuildingPlan : public Blueprint {
  public:
    // http://www.johnriebli.com/roof-types--house-styles.html
    enum RoofStyle {
        FLAT_ROOF = 0,
        HIPPED_ROOF,
        GABLED_ROOF,
        SAWTOOTH_ROOF,
        SHED_ROOF,
//        GAMBREL_ROOF,
    };

    static const std::vector<std::string> roofStyleNames()
    {
        return std::vector<std::string>({ "Flat", "Hipped", "Gabled", "Sawtooth", "Shed" /*, "Gambrel"*/ });
    }

    static ci::PolyLine2f triangle();
    static ci::PolyLine2f square();
    static ci::PolyLine2f rectangle( const uint16_t width, const uint16_t height );
    static ci::PolyLine2f lshape();
    static ci::PolyLine2f plus();
    static ci::PolyLine2f tee();
    static ci::PolyLine2f randomOutline();

    static BlueprintRef create( const ci::PolyLine2f &outline, uint8_t floors = 1,
        const RoofStyle roof = FLAT_ROOF, float slope = 0.5, float overhang = 0.0f )
    {
        return BlueprintRef( new BuildingPlan( outline, floors, roof, slope, overhang ) );
    }

    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    BuildingPlan( const ci::PolyLine2f &outline, uint8_t floors = 1,
        RoofStyle roof = FLAT_ROOF, float slope = 0.5, float overhang = 0.0f )
        : Blueprint( outline ), mRoof( roof ), mFloors( floors ), mRoofOverhang( overhang )
    {
        assert( mOutline.size() > 0 );

        // This assumes the caller actually sets the closed flag, and that we
        // always want a closed outline.
        if ( ! mOutline.isClosed() ) {
            mOutline.push_back( mOutline.getPoints().front() );
        }

        buildGeometry();
    };

    uint8_t floors() const { return mFloors; }
    float floorHeight() const { return mFloorHeight; }

  private:
    void buildGeometry();

    RoofStyle       mRoof;
    float           mRoofOverhang;
    float           mRoofSlope = 0.5;
    uint8_t         mFloors = 1;
    const float     mFloorHeight = 10.0;
};

class OilTank : public Blueprint {
  public:

    static BlueprintRef create( float radius = 20.0, float height = 10.0 )
    {
        return BlueprintRef( new OilTank( radius, height ) );
    }

    OilTank( float radius, float height, u_int8_t subdivisions = 12 )
        : Blueprint( polylineCircle( radius, subdivisions ) )
    {
        mGeometry = ci::geom::Cylinder().radius( radius ).height( height )
            .subdivisionsAxis( subdivisions ).direction( ci::vec3( 0, 0, 1 ) );
    }
};

class SmokeStack : public Blueprint {
  public:

    static BlueprintRef create( float radius = 4.0, float height = 40.0 )
    {
        return BlueprintRef( new SmokeStack( radius, height ) );
    }

    SmokeStack( float radius, float height, u_int8_t subdivisions = 12 )
        : Blueprint( polylineCircle( radius, subdivisions ) )
    {
        mGeometry = ci::geom::Cone().base( radius ).height( height ).apex( radius * 0.8 )
            .subdivisionsAxis( subdivisions ).direction( ci::vec3( 0, 0, 1 ) );
    }
};
