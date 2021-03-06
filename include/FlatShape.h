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

typedef std::shared_ptr<class cinder::TriMesh>		TriMeshRef;

typedef std::shared_ptr<class FlatShape>    	FlatShapeRef;

typedef std::pair<ci::vec2, ci::vec2> seg2;
typedef std::vector<ci::PolyLine2f> PolyLine2fs;

class FlatShape {
  public:

    static FlatShapeRef create( const ci::PolyLine2f &outline, const PolyLine2fs &holes = {} )
    {
        return std::make_shared<FlatShape>( outline, holes );
    }

    static FlatShapeRef create( const CGAL::Polygon_with_holes_2<ExactK> &pwh )
    {
        return std::make_shared<FlatShape>( pwh );
    }

    static FlatShapeRef create( const Arrangement_2::Face_iterator &face )
    {
        ci::PolyLine2f lotOutline = polyLineFrom( face->outer_ccb() );
        PolyLine2fs lotHoles;
        for ( auto hole = face->holes_begin(); hole != face->holes_end(); ++hole ) {
            lotHoles.push_back( polyLineFrom( *hole ) );
        }
        return std::make_shared<FlatShape>( lotOutline, lotHoles );
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

    std::vector<seg2>       edges() const;

    const ci::TriMeshRef    mesh() const;

    float       area() const { return mArea; }
    ci::vec2    centroid() const;
    ci::Rectf   boundingBox() const { return ci::Rectf( mOutline.getPoints() ); }
    ci::vec2    randomPoint() const;

    bool        contains( const ci::vec2 point ) const;

    std::vector<FlatShape>  contract( float amount ) const;

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

    Arrangement_2 arrangement() const;

    // Create a set of parallel lines that cross the shape, returns only the
    // segments that overlap the shape.
    std::vector<seg2>       dividerSeg2s( float angle, float spacing ) const;
    std::vector<Segment_2>  dividerSegment_2s( float angle, float spacing ) const;

    friend std::ostream& operator<<( std::ostream& lhs, const FlatShape& rhs )
    {
        lhs << "\nouter is ";
        bool colinear;
        if ( rhs.outline().isClockwise( &colinear ) ) {
            lhs << "clockwise";
        } else if ( colinear ) {
            lhs << "collinear";
        } else {
            lhs << "counter-clockwise";
        }
        lhs << "\n";
        // Print these manually so they are in a format that can be copy pasted
        // into Grapher for visual debugging.
        for ( const auto &point : rhs.outline() ) {
            lhs << point.x << ",\t" << point.y << "\n";
        }
        if( rhs.holes().size() > 0 ) {
            lhs << "has " << rhs.holes().size() << " holes: \n";
            for ( const ci::PolyLine2f &hole : rhs.holes() ) {
                lhs << "\t* " ;
                if ( hole.isClockwise( &colinear ) ) {
                    lhs << "clockwise";
                } else if ( colinear ) {
                    lhs << "collinear";
                } else {
                    lhs << "counter-clockwise";
                }
                lhs << ":\n";
                for ( const auto &point : hole ) {
                    lhs << point.x << ",\t" << point.y << "\n";
                }
                lhs << "\n";
            }
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
