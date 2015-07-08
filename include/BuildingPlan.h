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

class BuildingPlan;
typedef std::shared_ptr<BuildingPlan> BuildingPlanRef;

class BuildingPlan {
public:
    // http://www.johnriebli.com/roof-types--house-styles.html
    // If you change this update BuildingPlan::randomRoof().
    enum RoofStyle {
        FLAT_ROOF = 0,
        HIPPED_ROOF = 1,
        GABLED_ROOF = 2,
        SAWTOOTH_ROOF = 3,
        SHED_ROOF = 4,
        GAMBREL_ROOF = 5,
    };

    static const std::vector<std::string> roofStyleNames()
    {
        return std::vector<std::string>({ "Flat", "Hipped", "Gabled", "Sawtooth", "Shed", "Gambrel" });
    }

    static RoofStyle randomRoof();

    static ci::PolyLine2f triangle();
    static ci::PolyLine2f square();
    static ci::PolyLine2f rectangle( const uint16_t width, const uint16_t height );
    static ci::PolyLine2f lshape();
    static ci::PolyLine2f plus();
    static ci::PolyLine2f tee();
    static ci::PolyLine2f randomOutline();

    static BuildingPlanRef create( const ci::PolyLine2f &outline, const BuildingPlan::RoofStyle roof );
    static BuildingPlanRef createRandom();

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
    const float floorHeight() const { return mFloorHeight; }

private:
    void makeMesh();

    ci::PolyLine2f mOutline;
    RoofStyle mRoof;
    const float mFloorHeight = 10.0;
    ci::gl::VboMeshRef mRoofMeshRef;
    ci::gl::VboMeshRef mWallMeshRef;
};

#endif /* defined(__Cityscape__BuildingPlan__) */
