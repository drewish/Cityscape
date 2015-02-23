//
//  Block.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Block.h"
#include "Lot.h"

#include "cinder/app/AppNative.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/exceptions.h>



typedef CGAL::Exact_predicates_inexact_constructions_kernel K ;
typedef CGAL::Polygon_2<K>           Polygon_2 ;
typedef CGAL::Straight_skeleton_2<K> Ss ;
typedef boost::shared_ptr<Ss> SsPtr ;


ci::Vec2f vecFrom(K::Point_2 p)
{
	return ci::Vec2f(p.x(), p.y());
}

K::Point_2 pointFrom(ci::Vec2f p)
{
	return K::Point_2(p.x, p.y);
}

void Block::draw()
{
    gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
    gl::draw( outline );

    for( auto itL = lots.begin(); itL != lots.end(); ++itL ) {
        itL->draw();
    }
}

void Block::subdivide()
{
    Polygon_2 poly;

    // Assume the outline is closed and first == last.
    // Don't want to bother with less than a triangle.
    if (outline.size() < 4) return;

    // start at +1 to avoid overlapping last point
	for ( auto i = --outline.end(); i != outline.begin(); --i ) {
		poly.push_back(pointFrom(*i));
	}

//    ci::app::console() << outline << std::endl;
//    ci::app::console() << poly << "\t" << poly.orientation() << std::endl;

    try {
        SsPtr iss = CGAL::create_interior_straight_skeleton_2(poly);

        lots.clear();
        std::cout << "Faces:" << iss->size_of_faces() << std::endl;

/*
        Ss::Halfedge_handle start, edge;
        edge = start = iss->
        do {
            std::cout << edge->vertex()->point() << std::endl;
            lotOutline.push_back(vecFrom(edge->vertex()->point()));
            edge = edge->prev();
        } while (edge != start);
*/

        for( auto face = iss->faces_begin(); face != iss->faces_end(); ++face ) {
            std::cout << "----" << std::endl;

            PolyLine2f lotOutline;
            Ss::Halfedge_handle edge, start;

//            edge = start = face->halfedge()->opposite();
//            do {
//                std::cout << edge->vertex()->point() << "\t" << edge->is_border() << "\t" << edge->is_bisector() << std::endl;
//                lotOutline.push_back(vecFrom(edge->vertex()->point()));
//                edge = edge->prev();
//            } while (edge != start);

            //
            start = face->halfedge();
            edge = start;
            do {
                std::cout << edge->vertex()->point() << "\t" << edge->is_border() << "\t" << edge->is_bisector() << std::endl;
                lotOutline.push_back(vecFrom(edge->vertex()->point()));
                edge = edge->next();
            } while (edge != start);

            Lot l = Lot(lotOutline);
            lots.push_back(l);
        }
    }
    catch (CGAL::Precondition_exception e) {
        return; // TODO
    }

	std::vector<ci::PolyLine2f > ret;

	typedef CGAL::Straight_skeleton_2<K> Ss ;

	//	typedef typename Ss::Vertex_const_handle     Vertex_const_handle ;
	//	typedef typename Ss::Halfedge_const_handle   Halfedge_const_handle ;
	//	typedef typename Ss::Halfedge_const_iterator Halfedge_const_iterator ;

	//	Halfedge_const_handle null_halfedge ;
	//	Vertex_const_handle   null_vertex ;

	//	std::cout << "Straight skeleton with " << iss.size_of_vertices()
	//	<< " vertices, " << iss.size_of_halfedges()
	//	<< " halfedges and " << iss.size_of_faces()
	//	<< " faces" << std::endl ;

//	for ( auto i = iss->halfedges_begin(); i != iss->halfedges_end(); ++i )
//	{
//		ci::PolyLine2f poly;
//		poly.push_back(vecFrom(i->opposite()->vertex()->point()));
//		poly.push_back(vecFrom(i->vertex()->point()));
//		ret.push_back(poly);
//	}


}