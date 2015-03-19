//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"


#include "CinderCGAL.h"

#include <CGAL/linear_least_squares_fitting_2.h>

Vec2f getCentroid( PolyLine2f input )
{
    K::Point_2 centroid = K::Point_2(0, 0);
    K::Line_2 line;
    CGAL::Dimension_tag<0> dt;

    Polygon_2 p = polyFrom( input );
//    ci::app::console() << "Poly_2: " << p << std::endl;

    CGAL::linear_least_squares_fitting_2( p.vertices_begin(), p.vertices_end(), line, centroid, dt);

    return vecFrom( centroid );
}

void Lot::draw()
{
    gl::lineWidth( 1 );
    gl::color( ColorA( 0.2f, 1.0f, 1.0f, 1.0f ) );
    gl::draw( outline );
//    ci::app::console() << "shape: " << outline << std::endl;

//    Vec2f c1 = outline.centroid();
//    ci::app::console() << "o.c: " << c1 << std::endl;
//    gl::color( ColorA( mColor, 1.0f ) );
//    gl::drawSolidCircle( c1, 15);

//    Vec2f c2 = getCentroid( outline );
//    ci::app::console() << "g.o: " << c2 << std::endl;
//    gl::color( ColorA( mColor, 1.8f ) );
//    gl::lineWidth( 10 );
//    gl::drawStrokedCircle( c2, 15 );


    building.draw();
}

void Lot::place( const Building b ) {
    building = b;
    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration.
    building.outline.offset(getCentroid(outline));
}