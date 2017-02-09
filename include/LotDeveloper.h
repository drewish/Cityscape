//
//  LotDeveloper.h
//  Cityscape
//
//  Created by Andrew Morton on 3/26/16.
//
//

#pragma once

#include "CityData.h"
#include "BuildingPlan.h"

namespace Cityscape {

class LotDeveloper {
  public:
    virtual ~LotDeveloper() {};
    virtual bool isValidFor( LotRef &lot ) const { return true; }
    virtual void buildIn( LotRef &lot ) const {};
};

class ParkDeveloper : public LotDeveloper {
  public:
    virtual void buildIn( LotRef &lot ) const override;

  private:
    float mTreeCoverRatio;
};

class SingleFamilyHomeDeveloper : public LotDeveloper {
  public:
    SingleFamilyHomeDeveloper( const std::vector<SceneryRef> &plans )
        : mPlans( plans ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<SceneryRef> mPlans;
};

class WarehouseDeveloper : public LotDeveloper {
  public:
    WarehouseDeveloper( const std::vector<SceneryRef> &plans )
        : mPlans( plans ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<SceneryRef> mPlans;
};

class FullLotDeveloper : public LotDeveloper {
  public:
    FullLotDeveloper( RoofStyle roof ): mRoof( roof ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

    const RoofStyle mRoof;
};

class GroupDeveloper : public LotDeveloper {
  public:
    struct Item {
        Item( SceneryRef scenery, ci::vec3 position = ci::vec3( 0 ), float rotation = 0 )
            : scenery( scenery ), position( position ), rotation( rotation ) {};

        ci::mat4 transformation( const ci::mat4 &input = ci::mat4() ) const
        {
            return glm::rotate( glm::translate( input, position ), rotation, ci::vec3( 0, 0, 1 ) );
        }

        SceneryRef scenery = nullptr;
        ci::vec3 position = ci::vec3( 0 );
        float rotation = 0;
    };

    struct Group {
        Group( const std::vector<Item> &items );

        std::vector<Item>   items;
        ci::PolyLine2       hull;
    };

    void addGroup( const std::vector<Item> &items ) {
        mGroups.push_back( GroupDeveloper::Group( items ) );
    }

    virtual void buildIn( LotRef &lot ) const override;

    std::vector<Group> mGroups;
};

class SquareGridDeveloper : public LotDeveloper {
  public:
    SquareGridDeveloper( SceneryRef scenery, float rowSpacing, float structureSpacing, float angle = 0.0 )
    :   mScenery( scenery ), mRowSpacing( rowSpacing ), mStructureSpacing( structureSpacing ), mAngle( angle )
    {};

    virtual bool isValidFor( LotRef &lot )  const override;
    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mRowSpacing;
    const float mStructureSpacing;
    const SceneryRef mScenery;
};

class FarmOrchardDeveloper : public LotDeveloper {
  public:
    FarmOrchardDeveloper( float angle = 0.0, float spacing = 13.0, float diameter = 5.0f )
        : mAngle( angle ), mTreeSpacing( spacing ), mDiameter( diameter ) {};

    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mTreeSpacing;
    const float mDiameter;
};

class FarmFieldDeveloper : public LotDeveloper {
  public:
    FarmFieldDeveloper( float angle = 0.0, float rowSpacing = 10.0, float rowWidth = 5.0f )
        : mAngle( angle ), mRowSpacing( rowSpacing ), mRowWidth( rowWidth ) {};

    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mRowSpacing;
    const float mRowWidth;
};

}
