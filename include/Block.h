//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Block__
#define __Cityscape__Block__

#include "cinder/TriMesh.h"
#include "cinder/Triangulate.h"

class Lot;

class Block {
public:

    typedef std::vector<ci::PolyLine2f> PolyLine2fs;

    Block( const Block &src )
        : mId(src.mId), mOutline(src.mOutline), mHoles(src.mHoles), mLots(src.mLots), mMesh(src.mMesh)
    {}
    Block( const unsigned int bid, const ci::PolyLine2f outline, const PolyLine2fs holes = {} )
        : mId(bid), mOutline(outline), mHoles(holes)
    {
        ci::Triangulator triangulator;
        triangulator.addPolyLine( outline );
        for( auto it = holes.begin(); it != holes.end(); ++it ) {
            triangulator.addPolyLine( *it );
        }

        mMesh = triangulator.calcMesh();
    };

    void setup();
    void draw();
    void subdivide();
    void placeBuildings();

    const ci::PolyLine2f outline();
    const PolyLine2fs holes();

    unsigned int mId;
    ci::PolyLine2f mOutline;
    PolyLine2fs mHoles;
    std::vector<Lot> mLots;
    ci::TriMesh2d mMesh;

};

#endif /* defined(__Cityscape__Block__) */
