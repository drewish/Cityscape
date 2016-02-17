//
//  BuildingPlan.h
//  Cityscape
//
//  Created by andrew morton on 6/8/15.
//
//

#pragma once

#include "cinder/gl/Vbo.h"

class BuildingPlan;
typedef std::shared_ptr<BuildingPlan> BuildingPlanRef;

class BuildingPlan {
public:
    // http://www.johnriebli.com/roof-types--house-styles.html
    enum RoofStyle {
        RANDOM_ROOF = 0,
        FLAT_ROOF,
        HIPPED_ROOF,
        GABLED_ROOF,
        SAWTOOTH_ROOF,
        SHED_ROOF,
        GAMBREL_ROOF,
    };

    static const std::vector<std::string> roofStyleNames()
    {
        return std::vector<std::string>({ "Random", "Flat", "Hipped", "Gabled", "Sawtooth", "Shed", "Gambrel" });
    }

    static ci::PolyLine2f triangle();
    static ci::PolyLine2f square();
    static ci::PolyLine2f rectangle( const uint16_t width, const uint16_t height );
    static ci::PolyLine2f lshape();
    static ci::PolyLine2f plus();
    static ci::PolyLine2f tee();
    static ci::PolyLine2f randomOutline();

    static BuildingPlanRef create( const ci::PolyLine2f &outline, const BuildingPlan::RoofStyle roof, float overhang = 0.0f )
    {
        return BuildingPlanRef( new BuildingPlan( outline, roof, overhang ) );
    }
    static BuildingPlanRef createRandom()
    {
        return BuildingPlanRef( new BuildingPlan( randomOutline(), RANDOM_ROOF ) );
    }

    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    BuildingPlan( const ci::PolyLine2f &outline, const RoofStyle roof = FLAT_ROOF, float overhang = 0.0f )
        : mOutline( outline ), mRoof( roof ), mOverhang( overhang )
    {
        assert( mOutline.size() > 0 );

        // This assumes the caller actually sets the closed flag, and that we
        // always want a closed outline.
        if ( ! mOutline.isClosed() ) {
            mOutline.push_back( mOutline.getPoints().front() );
        }

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

    ci::PolyLine2f  mOutline;
    RoofStyle       mRoof;
    float           mOverhang;
    const float     mFloorHeight = 10.0;
    ci::gl::VboMeshRef mRoofMeshRef;
    ci::gl::VboMeshRef mWallMeshRef;
};
