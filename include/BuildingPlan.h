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

class BuildingPlan;
typedef std::shared_ptr<BuildingPlan> BuildingPlanRef;

class BuildingPlan {
public:
    // http://www.johnriebli.com/roof-types--house-styles.html
    // TODO: check coding style for capitalization
    enum RoofStyle {
        FLAT_ROOF = 0,
        HIPPED_ROOF,
        GABLED_ROOF,
        SAWTOOTH_ROOF,
        SHED_ROOF,
        GAMBREL_ROOF
    };

    static const std::vector<std::string> roofStyleNames() {
        return std::vector<std::string>({ "Flat", "Hipped",
            "Gabled", "Sawtooth", "Shed", "Gambrel" });
    }

    static ci::PolyLine2f triangle();
    static ci::PolyLine2f square();
    static ci::PolyLine2f rectangle( const uint16_t width, const uint16_t height );
    static ci::PolyLine2f lshape();
    static ci::PolyLine2f plus();
    static ci::PolyLine2f tee();
    static ci::PolyLine2f randomOutline();

    static BuildingPlan random( const RoofStyle roof = FLAT_ROOF );

    static BuildingPlanRef create( const ci::PolyLine2f &outline, const BuildingPlan::RoofStyle roof );
    static BuildingPlanRef createRandom( const BuildingPlan::RoofStyle roof );

    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    BuildingPlan( const ci::PolyLine2f &outline, const RoofStyle roof = FLAT_ROOF )
        : mOutline(outline), mRoof(roof)
    {
        assert( outline.size() );
        makeMesh();
    };
    BuildingPlan( const BuildingPlan &s )
        : mOutline(s.mOutline), mRoof(s.mRoof), mRoofMeshRef(s.mRoofMeshRef), mWallMeshRef(s.mWallMeshRef)
    { };


    const ci::gl::VboMeshRef roofMeshRef() const { return mRoofMeshRef; };
    const ci::gl::VboMeshRef wallMeshRef() const { return mWallMeshRef; };
    const ci::PolyLine2f outline(const ci::vec2 offset = glm::zero<ci::vec2>(), const float rotation = 0.0) const;

    // Needs to become private
    const float mFloorHeight = 10.0;

private:
    void makeMesh();

    ci::PolyLine2f mOutline;
    RoofStyle mRoof;
    ci::gl::VboMeshRef mRoofMeshRef;
    ci::gl::VboMeshRef mWallMeshRef;
};

#endif /* defined(__Cityscape__BuildingPlan__) */
