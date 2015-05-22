//
//  FlatShape.h
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#ifndef __Cityscape__FlatShape__
#define __Cityscape__FlatShape__

#include "cinder/TriMesh.h"
#include "cinder/Triangulate.h"
#include "CinderCGAL.h"

class FlatShape {
  public:
    typedef std::vector<ci::PolyLine2f> PolyLine2fs;

    FlatShape( const FlatShape &s )
        : mOutline(s.mOutline), mHoles(s.mHoles), mMesh(s.mMesh)
    {}
    FlatShape( const ci::PolyLine2f &outline, const PolyLine2fs &holes = {} )
        : mOutline(outline), mHoles(holes)
    {
        mMesh = makeMesh();
    };
    FlatShape( const Polygon_with_holes_2 &pwh )
    {
        mOutline = polyLineFrom( pwh.outer_boundary() );

        mHoles.reserve( pwh.number_of_holes() );
        for ( auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit ) {
            mHoles.push_back( polyLineFrom( *hit ) );
        }

        mMesh = makeMesh();
    };

    const ci::PolyLine2f outline() { return mOutline; }
    const PolyLine2fs holes() { return mHoles; }
    const ci::TriMesh2d mesh() { return mMesh; }

    const ci::Vec2f centroid();

    const Polygon_2 polygon();
    const Polygon_with_holes_2 polygon_with_holes();

  private:

    const ci::TriMesh2d makeMesh();

    ci::PolyLine2f mOutline;
    PolyLine2fs mHoles;
    ci::TriMesh2d mMesh;
};

#endif /* defined(__Cityscape__FlatShape__) */
