//
//  CityData.h
//  Cityscape
//
//  Created by Andrew Morton on 3/1/16.
//
//
#pragma once


namespace CityScape {

    struct Block;
    struct Lot;
    struct Tree;
    typedef std::shared_ptr<Block>  BlockRef;
    typedef std::shared_ptr<Lot>    LotRef;
    typedef std::shared_ptr<Tree>   TreeRef;

    struct CityModel {
        std::vector<BlockRef> blocks;
    };

    struct Block {
        std::vector<LotRef> lots;
    };

    struct Lot {
        std::vector<TreeRef> trees;
    };

    struct Tree {
        ci::vec3 position;
        float diameter;
    };
    typedef std::shared_ptr<Tree>   TreeRef;

}