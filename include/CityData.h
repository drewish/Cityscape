//
//  CityData.h
//  Cityscape
//
//  Created by Andrew Morton on 3/1/16.
//
//
#pragma once

class FlatShape;
typedef std::shared_ptr<FlatShape>  FlatShapeRef;

class Blueprint;
typedef std::shared_ptr<Blueprint>  BlueprintRef;

class Scenery;
typedef std::shared_ptr<Scenery>  SceneryRef;

class Scenery : public std::enable_shared_from_this<Scenery> {
  public:
    struct Instance {
        static ci::mat4 buildMatrix( const ci::vec3 &position = ci::vec3( 0 ), float rotation = 0 )
        {
            return glm::rotate( glm::translate( position ), rotation, ci::vec3( 0, 0, 1 ) );
        }
        // TODO: This should become a method of PolyLine now that it's only 2d.
        static ci::PolyLine2 transform( const ci::PolyLine2 &poly, const ci::mat4 &transformation )
        {
            ci::PolyLine2f result;
            for ( const auto &it : poly ) {
                result.push_back( ci::vec2( transformation * ci::vec4( it, 1, 1 ) ) );
            }
            return result;
        }

        Instance( const SceneryRef &scenery, const ci::vec2 &position = ci::vec2( 0 ), float rotation = 0, ci::ColorA color = ci::ColorA::white() )
            : scenery( scenery ), transformation( buildMatrix( ci::vec3( position, 0 ), rotation ) )
        {};
        Instance( const SceneryRef &scenery, const ci::vec3 &position = ci::vec3( 0 ), float rotation = 0, ci::ColorA color = ci::ColorA::white() )
            : scenery( scenery ), transformation( buildMatrix( position, rotation ) )
        {};
        Instance( const SceneryRef &scenery, const ci::mat4 &matrix, ci::ColorA color = ci::ColorA::white() )
            : scenery( scenery ), transformation( matrix ), color( color )
        {};

        // Footprint of the Scenery in its position on the lot
        ci::PolyLine2f footprint() const
        {
            return transform( scenery->footprint(), transformation );
        }

        const SceneryRef    scenery;
        ci::mat4            transformation;
        ci::ColorA          color;
    };
    Scenery( const ci::PolyLine2f &footprint, const ci::geom::SourceMods &geometry )
        : mFootprint( footprint ), mGeometry( geometry ) {};

    // 2d view of the object for collsion detection
    const ci::PolyLine2f footprint() const { return mFootprint; };
    // 3d view of the object for display
    const ci::geom::SourceMods &geometry() const { return mGeometry; };
    // Allow groups of other scenery to be placed in a lot together
    std::vector<Instance>& children() { return mChildren; }
    const std::vector<Instance>& children() const { return mChildren; }

    Scenery::Instance instance( const ci::vec2 &at, float rotation = 0 )
    {
        return Scenery::Instance( shared_from_this(), at, rotation );
    }
    Scenery::Instance instance( const ci::vec3 &at, float rotation = 0 )
    {
        return Scenery::Instance( shared_from_this(), at, rotation );
    }
  protected:
    ci::PolyLine2f          mFootprint;
    ci::geom::SourceMods    mGeometry;
    std::vector<Instance>   mChildren;
};

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

    typedef std::shared_ptr<LotDeveloper>   LotDeveloperRef;
    typedef std::shared_ptr<ZoningPlan>     ZoningPlanRef;
    typedef std::shared_ptr<Highway>        HighwayRef;
    typedef std::shared_ptr<Street>         StreetRef;
    typedef std::shared_ptr<District>       DistrictRef;
    typedef std::shared_ptr<Block>      	BlockRef;
    typedef std::shared_ptr<Lot>        	LotRef;

    struct ZoningPlan {
        static ZoningPlanRef create( const std::string &name ) {
            return std::make_shared<ZoningPlan>( name );
        };

        ZoningPlan( const std::string &name ) : name( name ) {};

        enum StreetDivision {
            NO_STREET_DIVISION = 0,
            GRID_STREET_DIVIDED = 1,
        };
        enum LotDivision {
            NO_LOT_DIVISION = 0,
            SKELETON_LOT_DIVISION = 1,
            OOB_LOT_DIVISION = 2,
        };

        std::string name;

        struct DistrictOptions {
            StreetDivision streetDivision = GRID_STREET_DIVIDED;
            struct GridOptions {
                uint8_t roadWidth = 8;
                int16_t avenueAngle = 0; // -180 - +180 degrees
                int16_t streetAngle = 90; // -90 - +90 degrees
                uint16_t avenueSpacing = 200;
                uint16_t streetSpacing = 300;
            } grid;
        } district;

        struct BlockOptions {
            LotDivision lotDivision = NO_LOT_DIVISION;
            uint16_t lotWidth = 40;
            uint32_t lotAreaMin = 1000;
            uint32_t lotAreaMax = 40000;
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

        void addUsage( const LotDeveloperRef &developer, uint8_t ratio = 1 ) {
            lotUsages.push_back( LotUsage( developer, ratio ) );
        }
    };

    // * * *

    struct CityModel {
        CityModel();
        CityModel( const std::vector<ZoningPlanRef> &zoning) : zoningPlans( zoning ) {}

        ci::Rectf   dimensions = ci::Rectf( -600, -600, 600, 600 );
        ci::Color   groundColor = ci::Color8u(233, 203, 151);

        ci::ColorA  roadColor = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f );
        uint8_t highwayWidth = 20;

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
            return std::make_shared<Highway>( from, to );
        };

        using Road::Road;

        uint8_t width = 20;
    };

    struct Street : public Road {
        static StreetRef create( const ci::vec2 &from, const ci::vec2 &to ) {
            return std::make_shared<Street>( from, to );
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
            return std::make_shared<District>( s, zp );
        };

        using Ground::Ground;
        District( const FlatShapeRef &s, const ZoningPlanRef &zp ) : Ground( s ), zoningPlan( zp ) {};

        ZoningPlanRef           zoningPlan;
        std::vector<BlockRef>   blocks;
    };

    struct Block : public Ground {
        static BlockRef create( const FlatShapeRef &s ) { return std::make_shared<Block>( s ); };

        using Ground::Ground;

        std::vector<LotRef>     lots;
    };

    struct Lot : public Ground {
        static LotRef create( const FlatShapeRef &s ) { return std::make_shared<Lot>( s ); };

        using Ground::Ground;

        std::vector<seg2> streetFacingSides;
        std::vector<Scenery::Instance> buildings;
        std::vector<Scenery::Instance> plants;
    };

} // Cityscape namespace
