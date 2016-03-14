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

    struct Highway;
    struct Street;
    struct District;
    struct Block;
    struct Lot;
    struct Building;
    struct Tree;
    typedef std::shared_ptr<Highway>    HighwayRef;
    typedef std::shared_ptr<Street>     StreetRef;
    typedef std::shared_ptr<District>   DistrictRef;
    typedef std::shared_ptr<Block>      BlockRef;
    typedef std::shared_ptr<Lot>        LotRef;
    typedef std::shared_ptr<Building>   BuildingRef;
    typedef std::shared_ptr<Tree>    	TreeRef;

    struct CityModel {
        CityModel() {}
        CityModel( const std::vector<ci::vec2> &highwayPoints );
        // For debugging build a city from a small portion
        CityModel( const BlockRef &block );
        CityModel( const BuildingRef &lot );

        Options options;

        std::vector<HighwayRef>     highways;
        std::vector<StreetRef>      streets;
        std::vector<FlatShapeRef>   pavement;

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
        _Ground( const FlatShapeRef &s, const ci::ColorA &c ) : shape( s ), color( c ) {};


        FlatShapeRef    shape;
        ci::ColorA      color;
    };

    struct District : public _Ground {
        static DistrictRef create( const FlatShapeRef &s ) { return DistrictRef( new District( s ) ); };
        static DistrictRef create( const FlatShapeRef &s, const ci::ColorA &c ) {
            return DistrictRef( new District( s, c ) );
        };

        using _Ground::_Ground;

        std::vector<BlockRef>   blocks;
    };

    struct Block : public _Ground {
        static BlockRef create( const FlatShapeRef &s ) { return BlockRef( new Block( s ) ); };
        static BlockRef create( const FlatShapeRef &s, const ci::ColorA &c ) {
            return BlockRef( new Block( s, c ) );
        };

        using _Ground::_Ground;

        std::vector<LotRef>     lots;
    };

    struct Lot : public _Ground {
        static LotRef create( const FlatShapeRef &s ) { return LotRef( new Lot( s ) ); };
        static LotRef create( const FlatShapeRef &s, const ci::ColorA &c ) {
            return LotRef( new Lot( s, c ) );
        };

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