//
//  FlatShape.h
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#pragma once

#include "cinder/TriMesh.h"
#include "CgalPolygon.h"
#include "CgalArrangement.h"

class FlatShape;
typedef std::shared_ptr<FlatShape>    	FlatShapeRef;

typedef std::pair<ci::vec2, ci::vec2> seg2;

class FlatShape {
  public:
    typedef std::vector<ci::PolyLine2f> PolyLine2fs;

    static FlatShapeRef create( const ci::PolyLine2f &outline, const PolyLine2fs &holes = {} )
    {
        return FlatShapeRef( new FlatShape( outline, holes ) );
    }

    static FlatShapeRef create( const CGAL::Polygon_with_holes_2<ExactK> &pwh )
    {
        return FlatShapeRef( new FlatShape( pwh ) );
    }

    // * * *


    FlatShape( const FlatShape &s )
        : mOutline( s.mOutline ), mHoles( s.mHoles ), mMesh( s.mMesh ), mArea( s.mArea )
    {}
    FlatShape( const ci::PolyLine2f &outline, const PolyLine2fs &holes = {} )
        : mOutline( outline ), mHoles( holes )
    {
        mArea = mOutline.calcArea();
        for ( const auto &hole : holes ) {
            mArea -= hole.calcArea();
        }

        mMesh = makeMesh();
    };
    FlatShape( const CGAL::Polygon_with_holes_2<ExactK> &pwh )
    {
        mOutline = polyLineFrom<ExactK>( pwh.outer_boundary() );
        mArea = mOutline.calcArea();

        mHoles.reserve( pwh.number_of_holes() );
        for ( auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit ) {
            mHoles.push_back( polyLineFrom<ExactK>( *hit ) );
            mArea -= mHoles.back().calcArea();
        }

        mMesh = makeMesh();
    };

    const ci::PolyLine2f outline() const { return mOutline; }
    const PolyLine2fs holes() const { return mHoles; }
    const ci::TriMesh mesh() const { return mMesh; }

    float       area() const { return mArea; }
    ci::vec2    centroid() const;
    ci::Rectf   boundingBox() const { return ci::Rectf( mOutline.getPoints() ); }
    ci::vec2    randomPoint() const;

    bool        contains( const ci::vec2 point ) const;

    template<class K>
    const CGAL::Polygon_2<K> polygon() const
    {
        return polygonFrom<K>( mOutline );
    }

    template<class K>
    const CGAL::Polygon_with_holes_2<K> polygonWithHoles() const
    {
        CGAL::Polygon_with_holes_2<K> poly( polygon<K>() );
        for ( auto &h : mHoles ) {
            poly.add_hole( polygonFrom<K>( h ) );
        }
        return poly;
    }

    const CGAL::Polygon_2<InexactK> polygonWithConnectedHoles() const;
    ci::PolyLine2f polyLineWithConnectedHoles() const;

    // Create a set of parallel lines that cross the shape, returns the segments
    // that overlap.
    std::vector<seg2>       dividerSeg2s( float angle, float spacing ) const;
    std::vector<Segment_2>  dividerSegment_2s( float angle, float spacing ) const;

  private:

    ci::TriMesh makeMesh() const;

    ci::PolyLine2f mOutline;
    PolyLine2fs mHoles;
    ci::TriMesh mMesh;
    float       mArea;
};
