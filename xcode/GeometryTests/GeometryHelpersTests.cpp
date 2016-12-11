//
//  GeometryHelpersTests.cpp
//  Cityscape
//
//  Created by andrew morton on 12/11/16.
//
//

#include "catch.hpp"
#include "cinder/Cinder.h"

#include "GeometryHelpers.h"

using namespace ci;

TEST_CASE( "contiguousSeg2sFrom", "[foo]" ) {
    SECTION( "0 points" ) {
        std::vector<vec2> input;
        std::vector<seg2> output;

        contiguousSeg2sFrom( input, back_inserter( output ) );

        REQUIRE( output.empty() );
    }

    SECTION( "1 point" ) {
        std::vector<vec2> input( { vec2( 0, 0 ) } );
        std::vector<seg2> output;

        contiguousSeg2sFrom( input, back_inserter( output ) );

        REQUIRE( output.empty() );
    }

    SECTION( "2 points" ) {
        std::vector<vec2> input( { vec2( 0, 0 ), vec2( 1, 1 ) } );
        std::vector<seg2> output;

        contiguousSeg2sFrom( input, back_inserter( output ) );

        REQUIRE( output.size() == 1 );
        REQUIRE( output.front() == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
    }

    SECTION( "3 points" ) {
        std::vector<vec2> input( { vec2( 0, 0 ), vec2( 1, 1 ), vec2( 2, 0 ) } );
        std::vector<seg2> output;

        contiguousSeg2sFrom( input, back_inserter( output ) );

        REQUIRE( output.size() == 2 );
        REQUIRE( output.front() == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
        REQUIRE( output.back() == seg2( vec2( 1, 1 ), vec2( 2, 0 ) ) );
    }
}

TEST_CASE( "pointsInPairs", "[foo]" ) {
    std::vector<seg2> output;
    std::function<void(const vec2&, const vec2&)> process = [&](const vec2 &a, const vec2 &b) { output.push_back( seg2( a, b ) ); };

    SECTION( "0 points" ) {
        PolyLine2f input;

        pointsInPairs( input, process );

        REQUIRE( output.empty() );
    }

    SECTION( "1 points" ) {
        PolyLine2f input( { vec2( 0, 0 ) } );

        pointsInPairs( input, process );

        REQUIRE( output.empty() );
    }

    SECTION( "2 points" ) {
        PolyLine2f input( { vec2( 0, 0 ), vec2( 1, 1 ) } );

        SECTION( "open" ) {
            pointsInPairs( input, process );

            REQUIRE( output.size() == 1 );
            REQUIRE( output.front() == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
        }

        SECTION( "closed" ) {
            input.setClosed();

            pointsInPairs( input, process );

            REQUIRE( output.size() == 2 );
            REQUIRE( output[0] == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
            REQUIRE( output[1] == seg2( vec2( 1, 1 ), vec2( 0, 0 ) ) );
        }
    }

    SECTION( "3 points, different first and last" ) {
        PolyLine2f input( { vec2( 0, 0 ), vec2( 1, 1 ), vec2( 2, 0 ) } );

        SECTION( "open" ) {
            pointsInPairs( input, process );

            REQUIRE( output.size() == 2 );
            REQUIRE( output[0] == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
            REQUIRE( output[1] == seg2( vec2( 1, 1 ), vec2( 2, 0 ) ) );
        }

        SECTION( "closed" ) {
            input.setClosed();

            pointsInPairs( input, process );

            REQUIRE( output.size() == 3 );
            REQUIRE( output[0] == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
            REQUIRE( output[1] == seg2( vec2( 1, 1 ), vec2( 2, 0 ) ) );
            REQUIRE( output[2] == seg2( vec2( 2, 0 ), vec2( 0, 0 ) ) );
        }
    }

    SECTION( "4 points, same first and last" ) {
        PolyLine2f input( { vec2( 0, 0 ), vec2( 1, 1 ), vec2( 2, 0 ), vec2( 0, 0 ) } );

        SECTION( "open" ) {
            pointsInPairs( input, process );

            REQUIRE( output.size() == 3 );
            REQUIRE( output[0] == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
            REQUIRE( output[1] == seg2( vec2( 1, 1 ), vec2( 2, 0 ) ) );
            REQUIRE( output[2] == seg2( vec2( 2, 0 ), vec2( 0, 0 ) ) );
        }

        SECTION( "closed" ) {
            input.setClosed();

            pointsInPairs( input, process );

            REQUIRE( output.size() == 3 );
            REQUIRE( output[0] == seg2( vec2( 0, 0 ), vec2( 1, 1 ) ) );
            REQUIRE( output[1] == seg2( vec2( 1, 1 ), vec2( 2, 0 ) ) );
            REQUIRE( output[2] == seg2( vec2( 2, 0 ), vec2( 0, 0 ) ) );
        }
    }

}

TEST_CASE( "anglesBetweenPointsIn", "[foo]" ) {
    std::vector<float> output;

    SECTION( "0 points" ) {
        PolyLine2f input;
        anglesBetweenPointsIn( input, std::back_inserter( output ) );

        REQUIRE( output.empty() );
    }

    SECTION( "1 points" ) {
        PolyLine2f input( { vec2( 0, 0 ) } );
        anglesBetweenPointsIn( input, std::back_inserter( output ) );

        REQUIRE( output.empty() );
    }

    SECTION( "2 points" ) {
        PolyLine2f input( { vec2( 0, 0 ), vec2( 1, 1 ) } );

        anglesBetweenPointsIn( input, std::back_inserter( output ) );

        REQUIRE( output.size() == 1 );
        REQUIRE( output[0] == Approx( M_PI_4 ) );
    }

    SECTION( "4 points, same first and last" ) {
        PolyLine2f input( { vec2( 0, 0 ), vec2( 1, 1 ), vec2( 2, 0 ), vec2( 0, 0 ) } );

        SECTION( "open" ) {
            anglesBetweenPointsIn( input, std::back_inserter( output ) );

            REQUIRE( output.size() == 3 );
            REQUIRE( output[0] == Approx( M_PI_4 ) );
            REQUIRE( output[1] == Approx( -M_PI_4 ) );
            REQUIRE( output[2] == Approx( M_PI ) );
        }

        SECTION( "closed" ) {
            input.setClosed();

            anglesBetweenPointsIn( input, std::back_inserter( output ) );

            REQUIRE( output.size() == 3 );
            REQUIRE( output[0] == Approx( M_PI_4 ) );
            REQUIRE( output[1] == Approx( -M_PI_4 ) );
            REQUIRE( output[2] == Approx( M_PI ) );
        }
    }

    SECTION( "4 points, different first and last" ) {
        PolyLine2f input( { vec2( 0, 0 ), vec2( 1, 1 ), vec2( 2, 0 ), vec2( 1, -3 ) } );

        SECTION( "open" ) {
            anglesBetweenPointsIn( input, std::back_inserter( output ) );

            REQUIRE( output.size() == 3 );
            REQUIRE( output[0] == Approx( atan2( 1, 1 ) ) );
            REQUIRE( output[1] == Approx( atan2( -1, 1) ) );
            REQUIRE( output[2] == Approx( atan2( -3, -1 ) ) );
        }

        SECTION( "closed" ) {
            input.setClosed();
            
            anglesBetweenPointsIn( input, std::back_inserter( output ) );

            REQUIRE( output.size() == 4 );
            REQUIRE( output[0] == Approx( atan2( 1, 1 ) ) );
            REQUIRE( output[1] == Approx( atan2( -1, 1) ) );
            REQUIRE( output[2] == Approx( atan2( -3, -1 ) ) );
            REQUIRE( output[3] == Approx( atan2( 3, -1 ) ) );
        }
    }

}
