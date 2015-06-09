//
//  BuildingPlan.h
//  Cityscape
//
//  Created by andrew morton on 6/8/15.
//
//

#ifndef __Cityscape__BuildingPlan__
#define __Cityscape__BuildingPlan__

#include "cinder/gl/Vbo.h"
#include "cinder/Rand.h"

class BuildingPlan {
public:
    // TODO: check coding style for capitalization
    // http://www.johnriebli.com/roof-types--house-styles.html
    enum RoofStyle {
        FLAT_ROOF = 0,
        HIPPED_ROOF,
        GABLED_ROOF,
        GAMBREL_ROOF,
        SHED_ROOF
    };

    static ci::PolyLine2f triangle() {
        return ci::PolyLine2f( {
            ci::Vec2f(10, -10), ci::Vec2f(10, 10), ci::Vec2f(-10, 10)
        } );
    }

    static ci::PolyLine2f square() {
        return ci::PolyLine2f( {
            ci::Vec2f(10, -10), ci::Vec2f(10, 10),
            ci::Vec2f(-10, 10), ci::Vec2f(-10, -10)
        } );
    }

    static ci::PolyLine2f lshape() {
        return ci::PolyLine2f( {
            ci::Vec2f(15, 0), ci::Vec2f(15, 10), ci::Vec2f(-15, 10),
            ci::Vec2f(-15, -10), ci::Vec2f(-5, -10), ci::Vec2f(-5, 0),
        } );
    }

    static ci::PolyLine2f plus() {
        return ci::PolyLine2f( {
            ci::Vec2f(15,-5), ci::Vec2f(15,5), ci::Vec2f(5,5),
            ci::Vec2f(5,15), ci::Vec2f(-5,15), ci::Vec2f(-5,5),
            ci::Vec2f(-15,5), ci::Vec2f(-15,-5), ci::Vec2f(-5,-5),
            ci::Vec2f(-5,-15), ci::Vec2f(5,-15), ci::Vec2f(5,-5),
        } );
    }

    static ci::PolyLine2f tee() {
        return ci::PolyLine2f( {
            ci::Vec2f(5,10), ci::Vec2f(-5,10), ci::Vec2f(-5,0),
            ci::Vec2f(-15,0), ci::Vec2f(-15,-10), ci::Vec2f(15,-10),
            ci::Vec2f(15,0), ci::Vec2f(5,0),
        } );
    }

    static ci::PolyLine2f randomOutline() {
        switch (ci::randInt(5)) {
            case 0:
                return triangle();
            case 1:
                return square();
            case 2:
                return lshape();
            case 3:
                return plus();
            default:
                return tee();

        }
    }

    static BuildingPlan random( const uint32_t floors, const RoofStyle roof ) {
        return BuildingPlan( randomOutline(), floors, roof );
    }

    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    BuildingPlan( const ci::PolyLine2f outline, const uint32_t floors = 1, const RoofStyle roof = FLAT_ROOF )
        : mOutline(outline), mFloors(floors), mRoof(roof)
    {
        mMeshRef = makeMesh();
    };
    BuildingPlan( const BuildingPlan &s )
        : mOutline(s.mOutline), mFloors(s.mFloors), mRoof(s.mRoof), mMeshRef(s.mMeshRef)
    { };


    const ci::gl::VboMeshRef meshRef() const { return mMeshRef; };
    const ci::PolyLine2f outline(const ci::Vec2f offset = ci::Vec2f::zero(), const float rotation = 0.0) const;

private:
    ci::gl::VboMeshRef makeMesh();

    ci::PolyLine2f mOutline;
    RoofStyle mRoof;
    uint32_t mFloors;
    ci::gl::VboMeshRef mMeshRef;

};

#endif /* defined(__Cityscape__BuildingPlan__) */
