#pragma once
#include "LeaderboardLayer.hpp"

using namespace geode::prelude;

class CreatorLeaderboardLayer : public LeaderboardLayer {
protected:
    const char* title() const override { return "K.D.L. Creator Leaderboard"; }
    const char* url() const override { return "https://the-kdl.com/api/cleaderboard"; }
    bool isCreator() const override { return true; }
    ccColor3B bgColor() const override { return {40, 0, 80}; }
 
 // hi what are you doing right now?
 // Are you either:
 // 1. Going through the codebase for fun
 // 2. "Borrowing" code for something of yours
 // 3. A index staff going through the codebase over a update submitted to be indexed
 
 // dm anticroom for either answer you picked :)

public:
    static CreatorLeaderboardLayer* create() {
        auto ret = new CreatorLeaderboardLayer();

        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    static CCScene* scene() {
        auto s = CCScene::create();
        s->addChild(CreatorLeaderboardLayer::create());
        return s;
    }
};
