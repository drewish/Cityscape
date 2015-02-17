//
//  Block.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Block.h"
#include "Lot.h"


#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/create_straight_skeleton_2.h>


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


void Block::subdivide()
{
	Polygon_2 poly;
	for ( auto i = outline.begin(); i != outline.end(); ++i ) {
		poly.push_back(pointFrom(*i));
	}

	// You can pass the polygon via an iterator pair
	SsPtr iss = CGAL::create_interior_straight_skeleton_2(poly);


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


	lots.clear();
	for( auto it = iss->faces_begin(); it != iss->faces_end(); ++it ) {
		// TODO: each face should become a lot
//		it->
		Lot l = Lot(outline);
		lots.push_back(l);
	}
}