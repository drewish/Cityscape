//
//  BuildingPlan.h
//  Cityscape
//
//  Created by andrew morton on 6/8/15.
//
//

#pragma once

#include "GeometryHelpers.h"
#include "CityData.h"

class BuildingPlan;
typedef std::shared_ptr<BuildingPlan>  BuildingPlanRef;

class BuildingPlan : public Scenery {
  public:
    // http://www.johnriebli.com/roof-types--house-styles.html
    enum RoofStyle {
        FLAT_ROOF = 0,
        HIPPED_ROOF,
        GABLED_ROOF,
        SAWTOOTH_ROOF,
        SHED_ROOF,
        //GAMBREL_ROOF,

        ROOF_STYLE_COUNT
    };

    static const std::vector<std::string> roofStyleNames()
    {
        return std::vector<std::string>({ "Flat", "Hipped", "Gabled", "Sawtooth", "Shed" /*, "Gambrel"*/ });
    }

    static ci::PolyLine2f randomOutline();

    static BuildingPlanRef create( const ci::PolyLine2f &outline, uint8_t floors = 1,
        const RoofStyle roof = FLAT_ROOF, float slope = 0.5, float overhang = 0.0f )
    {
        return BuildingPlanRef( new BuildingPlan( outline, buildGeometry( outline, floors, roof, slope, overhang ) ) );
    }
    static BuildingPlanRef create( const ci::PolyLine2f &outline, const ci::geom::SourceMods &geometry )
    {
        return BuildingPlanRef( new BuildingPlan( outline, geometry ) );
    }

    Scenery::InstanceRef createInstace( const ci::vec2 &at, float rotation = 0 )
    {
        return Scenery::InstanceRef( new Instance( shared_from_this(), at, 0 ) );
    }

  protected:
    static ci::TriMesh buildGeometry( const ci::PolyLine2f &outline, uint8_t floors, RoofStyle roof, float slope, float overhang );

    BuildingPlan( const ci::PolyLine2f &outline, const ci::geom::SourceMods &geometry )
      : Scenery( outline, geometry ) {}

    struct Instance : public Scenery::Instance {
        Instance( const SceneryRef &plan, const ci::vec2 &at, float rotation )
        :   Scenery::Instance( plan, ci::vec3( at, 0 ), ci::Color::white() ),
            rotation( rotation )
        {};

        virtual ci::mat4 modelViewMatrix() const override
        {
            return glm::rotate( glm::translate( position ), rotation, ci::vec3( 0, 0, 1 ) );
        }

        float rotation; // radians
    };
};

ci::TriMesh buildingWithFlatRoof( const ci::PolyLine2f &footprint, float wallHeight, float overhang );
ci::TriMesh buildingWithHippedRoof( const ci::PolyLine2f &footprint, float wallHeight, float slope, float overhang );
ci::TriMesh buildingWithGabledRoof( const ci::PolyLine2f &footprint, float wallHeight, float slope, float overhang );
ci::TriMesh buildingWithShedRoof( const ci::PolyLine2f &footprint, float wallHeight, float slope, float overhang );
struct SawtoothSettings {
    float downWidth;
    float upWidth;
    float valleyHeight;
    float peakHeight;
    float overhang;
};
ci::TriMesh buildingWithSawtoothRoof( const ci::PolyLine2f &wallOutline, const SawtoothSettings &settings );

