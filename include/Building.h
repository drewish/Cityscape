//
//  Building.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Building__
#define __Cityscape__Building__

#include "cinder/gl/Vbo.h"
#include "cinder/Rand.h"
#include "Options.h"

class Building {
  public:

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


    // TODO: check coding style for capitalization
    // http://www.johnriebli.com/roof-types--house-styles.html
    enum RoofStyle {
        NONE = 0,
        FLAT,
        HIPPED,
        GABLED,
        GAMBREL,
        SHED
    };

    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    // Default to a 10x10 square
    Building() : mOutline( square() ) {};
    Building( const ci::PolyLine2f outline, const ci::Color &c ) : mOutline(outline), mColor(c) { };
    Building( const Building &s ) : mColor(s.mColor), mOutline(s.mOutline), mFloors(s.mFloors), mRoof(s.mRoof) { };

    void layout();
    void draw( const Options &options );

    ci::PolyLine2f outline(const ci::Vec2f offset = ci::Vec2f::zero(), const float rotation = 0.0);

    ci::ColorA mColor = ci::ColorA(0.25, 0.5, 0.25, 0.25);
    ci::PolyLine2f mOutline;
    RoofStyle mRoof = HIPPED;
    uint32_t mFloors = 2;
    ci::gl::VboMesh mMesh;
    float mArea = 0;

  private:
    ci::gl::VboMesh makeMesh(const RoofStyle roof, const ci::PolyLine2f &outline, unsigned int floors);

};

#endif /* defined(__Cityscape__Building__) */
