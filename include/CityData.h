//
//  CityData.h
//  Cityscape
//
//  Created by Andrew Morton on 3/1/16.
//
//
#pragma once

// Should go someplace else...
#include "BuildingPlan.h"

class FlatShape;
typedef std::shared_ptr<FlatShape>    	FlatShapeRef;


struct RoadOptions {
    ci::ColorA color = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f );
    uint8_t highwayWidth = 40;
    uint8_t sidestreetWidth = 20;
    int16_t sidestreetAngle1 = 0; // -180 - +180 degrees
    int16_t sidestreetAngle2 = 90; // -90 - +90 degrees
    uint16_t blockHeight = 300;
    uint16_t blockWidth = 200;
};

struct BlockOptions {
    enum BlockDivision {
        NO_BLOCK_DIVISION = 0,
        BLOCK_DIVIDED = 1,
    };

    BlockDivision division = BLOCK_DIVIDED;
    int16_t lotWidth = 40;
};

struct LotOptions {
    enum BuildingPlacement {
        BUILDING_IN_CENTER = 0,
        BUILDING_FILL_LOT = 1,
    };

    BuildingPlacement buildingPlacement = BUILDING_IN_CENTER;
};

struct BuildingOptions {
    int roofStyle = 1;
};

struct Options {
    RoadOptions road;
    BlockOptions block;
    LotOptions lot;
    BuildingOptions building;
};

namespace Cityscape {
    // Give relatively unique colors
    ci::ColorA colorWheel();

    struct ZoningPlan;
    struct Highway;
    struct Street;
    struct District;
    struct Block;
    struct Lot;
    struct Building;
    struct Tree;

    typedef std::shared_ptr<ZoningPlan> ZoningPlanRef;
    typedef std::shared_ptr<Highway>    HighwayRef;
    typedef std::shared_ptr<Street>     StreetRef;
    typedef std::shared_ptr<District>   DistrictRef;
    typedef std::shared_ptr<Block>      BlockRef;
    typedef std::shared_ptr<Lot>        LotRef;
    typedef std::shared_ptr<Building>   BuildingRef;
    typedef std::shared_ptr<Tree>    	TreeRef;

    struct ZoningPlan {
        static ZoningPlanRef create( const std::string &name ) {
            return ZoningPlanRef( new ZoningPlan( name ) );
        };

        ZoningPlan( const std::string &name ) : name( name ) {};

        enum StreetDivision {
            NO_STREET_DIVISION = 0,
            GRID_STREET_DIVIDED = 1,
        };
        enum LotDivision {
            NO_LOT_DIVISION = 0,
            SKELETON_LOT_DIVISION = 1,
        };

        std::string name;

        struct RoadOptions {
            ci::ColorA color = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f );
            uint8_t highwayWidth = 40;
            uint8_t streetWidth = 20;
        } road;

        struct DistrictOptions {
            StreetDivision streetDivision = GRID_STREET_DIVIDED;
            struct GridOptions {
                int16_t avenueAngle = 0; // -180 - +180 degrees
                int16_t streetAngle = 90; // -90 - +90 degrees
                uint16_t avenueSpacing = 200;
                uint16_t streetSpacing = 300;
            } grid;
        } district;

        struct BlockOptions {
            LotDivision lotDivision = SKELETON_LOT_DIVISION;
            int16_t lotWidth = 40;
        } block;

        //- lotTypes (array of LotType)
        //    - ObjectType/Name: This might needs to be a class or reference to a class so it can have more high level behavior, e.g. a park that has options for tree coverage, reducing the Lot’s outline to create setbacks, etc.
        //    - Probability

        //- buildingPlans (array of BuildingPlan)
        //    - ObjectType/Name: This might needs to be a class or reference to a class so we can have look at the Lot and figure out where to place ourselves. Also should probably handle creating the actual mesh.
        //    - Probability
    };

    // * * *

    struct CityModel {
        CityModel() {}
        CityModel( const std::vector<ci::vec2> &highwayPoints );
        // For debugging build a city from a small portion
        CityModel( const BlockRef &block );
        CityModel( const BuildingRef &lot );

        Options     options;
        ci::Color   color = ci::Color8u(233, 203, 151);

        std::vector<HighwayRef>     highways;
        std::vector<StreetRef>      streets;
        std::vector<FlatShapeRef>   pavement;

        std::vector<ZoningPlanRef>  zoningPlans = { ZoningPlan::create( "default" ) };
        std::vector<DistrictRef>    districts;
    };

    // * * *

    struct Road {
        Road( const ci::vec2 &from, const ci::vec2 &to ) : centerline( { from, to } ) {};

        uint8_t width;
        ci::PolyLine2f centerline;
    };

    struct Highway : public Road {
        static HighwayRef create( const ci::vec2 &from, const ci::vec2 &to ) {
            return HighwayRef( new Highway( from, to ) );
        };

        using Road::Road;

        uint8_t width = 20;
    };

    struct Street : public Road {
        static StreetRef create( const ci::vec2 &from, const ci::vec2 &to ) {
            return StreetRef( new Street( from, to ) );
        };

        using Road::Road;

        uint8_t width = 10;
    };

    // * * *

    // TODO Need a better name for this
    struct _Ground {
        _Ground( const FlatShapeRef &s ) : shape( s ), color( colorWheel() ) {};

        FlatShapeRef    shape;
        ci::ColorA      color;
    };

    struct District : public _Ground {
        static DistrictRef create( const FlatShapeRef &s, const ZoningPlanRef &zp )
        {
            return DistrictRef( new District( s, zp ) );
        };

        using _Ground::_Ground;
        District( const FlatShapeRef &s, const ZoningPlanRef &zp ) : _Ground( s ), zoningPlan( zp ) {};

        ZoningPlanRef           zoningPlan;
        std::vector<BlockRef>   blocks;
    };

    struct Block : public _Ground {
        static BlockRef create( const FlatShapeRef &s ) { return BlockRef( new Block( s ) ); };

        using _Ground::_Ground;

        std::vector<LotRef>     lots;
    };

    struct Lot : public _Ground {
        static LotRef create( const FlatShapeRef &s ) { return LotRef( new Lot( s ) ); };

        using _Ground::_Ground;

        std::vector<TreeRef>    trees;
        BuildingRef             building;
    };

    struct Building {
        static BuildingRef create( const BuildingPlan &plan, uint8_t floors = 1, ci::vec2 position = ci::vec2( 0, 0 ), float rotation = 0 )
        {
            return BuildingRef( new Building( plan, floors, position, rotation ) );
        }

        Building( const BuildingPlan &plan, uint8_t floors = 1, ci::vec2 position = ci::vec2( 0, 0 ), float rotation = 0 )
            : plan( plan ), floors( floors ), position( position ), rotation( rotation ) {};

        BuildingPlan    plan;
        uint8_t         floors;
        ci::vec2        position;
        float           rotation; // radians
    };

    struct Tree {
        static TreeRef create( const ci::vec3 &p, float d ) { return TreeRef( new Tree( p, d ) ); };

        Tree( const ci::vec3 &position, float diameter ) : position( position ), diameter( diameter ) {};

        ci::vec3 position;
        float diameter;
    };
    typedef std::shared_ptr<Tree>   TreeRef;

} // Cityscape namespace