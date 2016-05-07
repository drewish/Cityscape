//
//  FlatShape.h
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#pragma once

#include "CgalPolygon.h"
#include "CgalArrangement.h"

class TriMesh;
typedef std::shared_ptr<TriMesh>    	TriMeshRef;

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
        fixUp();
        mArea = calcArea();
    };
    FlatShape( const CGAL::Polygon_with_holes_2<ExactK> &pwh )
        : mOutline( polyLineFrom<ExactK>( pwh.outer_boundary() ) )
    {
        mHoles.reserve( pwh.number_of_holes() );
        for ( auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit ) {
            mHoles.push_back( polyLineFrom<ExactK>( *hit ) );
        }
        fixUp();
        mArea = calcArea();
    };
    FlatShape( const CGAL::Polygon_with_holes_2<InexactK> &pwh )
        : mOutline( polyLineFrom<InexactK>( pwh.outer_boundary() ) )
    {
        mHoles.reserve( pwh.number_of_holes() );
        for ( auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit ) {
            mHoles.push_back( polyLineFrom<InexactK>( *hit ) );
        }
        fixUp();
        mArea = calcArea();
    };

    const ci::PolyLine2f    outline() const { return mOutline; }
    const PolyLine2fs       holes() const { return mHoles; }

    const ci::TriMeshRef    mesh() const;

    float       area() const { return mArea; }
    ci::vec2    centroid() const;
    ci::Rectf   boundingBox() const { return ci::Rectf( mOutline.getPoints() ); }
    ci::vec2    randomPoint() const;

    bool        contains( const ci::vec2 point ) const;

    std::vector<FlatShape>  contract( double amount ) const;

    template<class K>
    const CGAL::Polygon_2<K> polygon() const
    {
        return polygonFrom<K>( mOutline );
    }

    template<class K>
    const CGAL::Polygon_with_holes_2<K> polygonWithHoles() const
    {
        CGAL::Polygon_with_holes_2<K> poly( polygonFrom<K>( mOutline ) );
        for ( const auto &h : mHoles ) {
            poly.add_hole( polygonFrom<K>( h ) );
        }
        return poly;
    }

    CGAL::Polygon_2<InexactK> polygonWithConnectedHoles() const;
    ci::PolyLine2f polyLineWithConnectedHoles() const;

    // Create a set of parallel lines that cross the shape, returns the segments
    // that overlap.
    std::vector<seg2>       dividerSeg2s( float angle, float spacing ) const;
    std::vector<Segment_2>  dividerSegment_2s( float angle, float spacing ) const;

	friend std::ostream& operator<<( std::ostream& lhs, const FlatShape& rhs )
	{
        lhs << "\nouter is ";
        switch ( rhs.outline().orientation() ) {
            case ci::PolyLine2f::COLLINEAR:
                lhs << "collinear";
                break;
            case ci::PolyLine2f::COUNTER_CLOCKWISE:
                lhs << "counter-";
            case ci::PolyLine2f::CLOCKWISE:
                lhs << "clockwise";
                break;
        }
        lhs << " with " << rhs.holes().size() << " holes:\n" << rhs.outline();
        for ( const ci::PolyLine2f &hole : rhs.holes() ) {
            lhs << "hole with " << hole.size() << " points going ";
            if ( hole.isCounterClockwise() ) {
                lhs << "counter-";
            }
            lhs << "clockwise:\n" << hole << "\n";
        }
        return lhs;
    }

  private:

    void    fixUp();
    float   calcArea() const;

    ci::PolyLine2f          mOutline;
    PolyLine2fs             mHoles;
    float                   mArea;
    mutable ci::TriMeshRef  mMesh;
};
