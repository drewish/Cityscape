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

class FlatShape {
  public:
    typedef std::vector<ci::PolyLine2f> PolyLine2fs;

    FlatShape( const FlatShape &s )
        : mOutline(s.mOutline), mHoles(s.mHoles), mMesh(s.mMesh)
    {}
    FlatShape( const ci::PolyLine2f outline, const PolyLine2fs holes = {} )
        : mOutline(outline), mHoles(holes)
    {
        // TODO might be good to lazily create this when they first ask for the mesh.
        ci::Triangulator triangulator;
        triangulator.addPolyLine( outline );
        for( auto it = holes.begin(); it != holes.end(); ++it ) {
            triangulator.addPolyLine( *it );
        }

        mMesh = triangulator.calcMesh();
    };

    const ci::PolyLine2f outline() { return mOutline; }
    const PolyLine2fs holes() { return mHoles; }
    const ci::TriMesh2d mesh() { return mMesh; }

    const ci::Vec2f centroid();

  private:
    ci::PolyLine2f mOutline;
    PolyLine2fs mHoles;
    ci::TriMesh2d mMesh;
};

#endif /* defined(__Cityscape__FlatShape__) */
