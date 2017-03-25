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

// http://www.johnriebli.com/roof-types--house-styles.html
enum class RoofStyle {
    FLAT = 0,
    HIPPED,
    GABLED,
    SAWTOOTH,
    SHED,
    //GAMBREL,

    COUNT
};

ci::TriMesh buildingWithFlatRoof( const ci::PolyLine2f &footprint, float maxWallHeight, float minWallHeight, float overhang );
ci::TriMesh buildingWithHippedRoof( const ci::PolyLine2f &footprint, float maxWallHeight, float minWallHeight, float slope, float overhang );
ci::TriMesh buildingWithGabledRoof( const ci::PolyLine2f &footprint, float maxWallHeight, float minWallHeight, float slope, float overhang );
ci::TriMesh buildingWithShedRoof( const ci::PolyLine2f &footprint, float maxWallHeight, float slope, float overhang );
struct SawtoothSettings {
    float downWidth;
    float upWidth;
    float minWallHeight;
    float valleyHeight;
    float peakHeight;
    float overhang;
};
ci::TriMesh buildingWithSawtoothRoof( const ci::PolyLine2f &wallOutline, const SawtoothSettings &settings );

struct BuildingSettings {
    uint8_t floors = 1;
    RoofStyle roofStyle = RoofStyle::FLAT;
    float slope = 0.5f;
    float overhang = 0.0f;
};
ci::geom::SourceMods buildingGeometry( const ci::PolyLine2f &outline, const BuildingSettings &settings );


class BuildingPlan : public Scenery {
  public:
    static SceneryRef create( const ci::PolyLine2f &outline, uint8_t floors = 1, RoofStyle roofStyle = RoofStyle::FLAT, float slope = 0.5f, float overhang = 0.0f )
    {
        BuildingSettings settings;
        settings.floors = floors;
        settings.roofStyle = roofStyle;
        settings.slope = slope;
        settings.overhang = overhang;
        return SceneryRef( new BuildingPlan( outline, buildingGeometry( outline, settings) ) );
    }

    static SceneryRef create( const ci::PolyLine2f &outline, const BuildingSettings &settings )
    {
        return SceneryRef( new BuildingPlan( outline, buildingGeometry( outline, settings) ) );
    }

    BuildingPlan( const ci::PolyLine2f &outline, const ci::geom::SourceMods &geometry )
      : Scenery( outline, geometry ) {}
};

// Builds a layer cake style of building given a foot print then list of heights and contractions for the next layer
typedef std::vector<std::pair<float, float>> SetBackPlan;
ci::geom::SourceMods weddingCake( const ci::PolyLine2f &footprint, const SetBackPlan &heightAndContraction );
