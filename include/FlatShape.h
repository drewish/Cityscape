//
//  FlatShape.h
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#pragma once

#include "cinder/TriMesh.h"
#include "cinder/Triangulate.h"
#include "CinderCGAL.h"

class FlatShape {
  public:
    typedef std::vector<ci::PolyLine2f> PolyLine2fs;

    FlatShape( const FlatShape &s )
        : mOutline( s.mOutline ), mHoles( s.mHoles ), mMesh( s.mMesh ), mArea( s.mArea )
    {}
    FlatShape( const ci::PolyLine2f &outline, const PolyLine2fs &holes = {} )
        : mOutline( outline ), mHoles( holes )
    {
        mMesh = makeMesh();
        // TODO: This isn't subtracting out the area of holes
        mArea = mOutline.calcArea();
    };
    FlatShape( const CGAL::Polygon_with_holes_2<ExactK> &pwh )
    {
        mOutline = polyLineFrom<ExactK>( pwh.outer_boundary() );

        mHoles.reserve( pwh.number_of_holes() );
        for ( auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit ) {
            mHoles.push_back( polyLineFrom<ExactK>( *hit ) );
        }

        mMesh = makeMesh();
        // TODO: This isn't subtracting out the area of holes
        mArea = mOutline.calcArea();
    };

    const ci::PolyLine2f outline() const { return mOutline; }
    const PolyLine2fs holes() const { return mHoles; }
    const ci::TriMesh mesh() const { return mMesh; }

    float       area() const { return mArea; }
    ci::vec2    centroid() const;
    ci::Rectf   boundingBox() const { return ci::Rectf( mOutline.getPoints() ); }
    ci::vec2    randomPoint() const;

    template<class K>
    const CGAL::Polygon_2<K> polygon() const
    {
        return polygonFrom<K>( mOutline );
    }

    template<class K>
    const CGAL::Polygon_with_holes_2<K> polygon_with_holes() const
    {
        CGAL::Polygon_with_holes_2<K> poly( polygon<K>() );
        for ( auto &it : mHoles ) {
            poly.add_hole( polygonFrom<K>( it ) );
        }
        return poly;
    }
    
  private:

    const ci::TriMesh makeMesh();

    ci::PolyLine2f mOutline;
    PolyLine2fs mHoles;
    ci::TriMesh mMesh;
    float       mArea;
};
