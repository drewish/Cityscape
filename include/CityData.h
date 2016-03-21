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
typedef std::shared_ptr<FlatShape> FlatShapeRef;

namespace Cityscape {
    // Give relatively unique colors
    ci::ColorA colorWheel();

    class  LotDeveloper;
    struct ZoningPlan;
    struct Highway;
    struct Street;
    struct District;
    struct Block;
    struct Lot;
    struct Building;
    struct Tree;

    typedef std::shared_ptr<LotDeveloper>   LotDeveloperRef;
    typedef std::shared_ptr<ZoningPlan>     ZoningPlanRef;
    typedef std::shared_ptr<Highway>        HighwayRef;
    typedef std::shared_ptr<Street>         StreetRef;
    typedef std::shared_ptr<District>       DistrictRef;
    typedef std::shared_ptr<Block>      	BlockRef;
    typedef std::shared_ptr<Lot>        	LotRef;
    typedef std::shared_ptr<Building>   	BuildingRef;
    typedef std::shared_ptr<Tree>    		TreeRef;


    class LotDeveloper {
      public:
        virtual ~LotDeveloper() {};

        virtual const std::string name() const { return "Un-developer"; }
        virtual bool isValidFor( LotRef &lot ) const { return false; }
        virtual void buildIn( LotRef &lot ) const {};
    };

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

        struct DistrictOptions {
            StreetDivision streetDivision = GRID_STREET_DIVIDED;
            struct GridOptions {
                uint8_t roadWidth = 20;
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

        // TODO Think of a better name for this
        struct LotUsage {
            // No developer means empty lot
            LotUsage( const LotDeveloperRef &developer, uint8_t ratio = 1 )
                : developer( developer ), ratio( ratio ) {};
            LotDeveloperRef developer;
            uint8_t         ratio = 1;
        };
        std::vector<LotUsage> lotUsages;
    };

    // * * *

    struct CityModel {
        CityModel();
        CityModel( const std::vector<ZoningPlanRef> &zoning) : zoningPlans( zoning ) {}

        ci::Rectf   dimensions = ci::Rectf( -600, -600, 600, 600 );
        ci::Color   groundColor = ci::Color8u(233, 203, 151);

        ci::ColorA  roadColor = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f );
        uint8_t highwayWidth = 40;

        std::vector<HighwayRef>     highways;
        std::vector<StreetRef>      streets;
        std::vector<FlatShapeRef>   pavement;

        std::vector<ZoningPlanRef>  zoningPlans;
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
    struct Ground {
        Ground( const FlatShapeRef &s ) : shape( s ), color( colorWheel() ) {};

        FlatShapeRef    shape;
        ci::ColorA      color;
    };

    struct District : public Ground {
        static DistrictRef create( const FlatShapeRef &s, const ZoningPlanRef &zp )
        {
            return DistrictRef( new District( s, zp ) );
        };

        using Ground::Ground;
        District( const FlatShapeRef &s, const ZoningPlanRef &zp ) : Ground( s ), zoningPlan( zp ) {};

        ZoningPlanRef           zoningPlan;
        std::vector<BlockRef>   blocks;
    };

    struct Block : public Ground {
        static BlockRef create( const FlatShapeRef &s ) { return BlockRef( new Block( s ) ); };

        using Ground::Ground;

        std::vector<LotRef>     lots;
    };

    struct Lot : public Ground {
        static LotRef create( const FlatShapeRef &s ) { return LotRef( new Lot( s ) ); };

        using Ground::Ground;

        std::vector<TreeRef>    trees;
        BuildingRef             building;
    };

    struct Building {
        static BuildingRef create( const BuildingPlanRef &plan, ci::vec2 position = ci::vec2( 0, 0 ), float rotation = 0 )
        {
            return BuildingRef( new Building( plan, position, rotation ) );
        }

        Building( const BuildingPlanRef &plan, ci::vec2 position = ci::vec2( 0, 0 ), float rotation = 0 )
            : plan( plan ), position( position ), rotation( rotation ) {};

        BuildingPlanRef plan;
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