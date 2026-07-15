#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

inline void addKDLBackground(CCNode* target, ccColor3B color, float speed = 15.0f) {
    auto winSize = CCDirector::get()->getWinSize();

    ccColor3B lightColor = {
        static_cast<GLubyte>(std::min(255, color.r + 60)),
        static_cast<GLubyte>(std::min(255, color.g + 60)),
        static_cast<GLubyte>(std::min(255, color.b + 60))
    };

    auto bgContainer = CCNode::create();
    bgContainer->setAnchorPoint({0.0f, 0.0f});
    bgContainer->setID("scrolling-background");

    ccTexParams bgParams = {GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE};

    auto bgSample = CCSprite::create("KDL_game_bg_01_001.png"_spr);
    float bgWidth = bgSample->getContentSize().width;
    float bgHeight = bgSample->getContentSize().height;
    float bgScale = (winSize.height / bgHeight) * 1.35f;
    float tileWidth = bgWidth * bgScale;
    float tilePeriod = tileWidth * 2.0f;

    int tileCount = static_cast<int>(std::ceil(winSize.width / tileWidth)) + 3;

    auto makeTile = [&](int index, float xPos) {
        std::string tex = (index % 2 == 0) ? "KDL_game_bg_01_001.png"_spr : "KDL_game_bg_01_002.png"_spr;

        auto tile = CCSprite::create(tex.c_str());
        tile->setColor(lightColor);
        tile->getTexture()->setTexParameters(&bgParams);
        tile->setAnchorPoint({0.0f, 0.0f});
        tile->setScale(bgScale);
        tile->setPosition({xPos, 0.0f});
        bgContainer->addChild(tile);

        auto mirroredTop = CCSprite::create(tex.c_str());
        mirroredTop->setColor(lightColor);
        mirroredTop->getTexture()->setTexParameters(&bgParams);
        mirroredTop->setAnchorPoint({0.0f, 0.0f});
        mirroredTop->setScale(bgScale);
        mirroredTop->setPosition({xPos, bgHeight * bgScale});
        mirroredTop->setFlipY(true);
        bgContainer->addChild(mirroredTop);

        auto mirroredBottom = CCSprite::create(tex.c_str());
        mirroredBottom->setColor(lightColor);
        mirroredBottom->getTexture()->setTexParameters(&bgParams);
        mirroredBottom->setAnchorPoint({0.0f, 0.0f});
        mirroredBottom->setScale(bgScale);
        mirroredBottom->setPosition({xPos, -bgHeight * bgScale});
        mirroredBottom->setFlipY(true);
        bgContainer->addChild(mirroredBottom);
    };

    for (int i = 0; i < tileCount; i++) {
        makeTile(i, i * tileWidth);
    }

    float yOffset = -(bgHeight * bgScale - winSize.height) / 2.0f;
    bgContainer->setPosition({0.0f, yOffset});
    target->addChild(bgContainer, -2);

    bgContainer->runAction(CCRepeatForever::create(CCSequence::create(
        CCMoveBy::create(speed * (tilePeriod / winSize.width), ccp(-tilePeriod, 0)),
        CCMoveTo::create(0.0f, ccp(0, yOffset)),
        nullptr
    )));

    auto bottomLeftCorner = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    bottomLeftCorner->setPosition({-1.0f, -1.0f});
    bottomLeftCorner->setAnchorPoint({0.0f, 0.0f});
    bottomLeftCorner->setID("left-corner");
    target->addChild(bottomLeftCorner, -1);

    auto bottomRightCorner = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    bottomRightCorner->setPosition({winSize.width + 1.0f, -1.0f});
    bottomRightCorner->setAnchorPoint({1.0f, 0.0f});
    bottomRightCorner->setFlipX(true);
    bottomRightCorner->setID("right-corner");
    target->addChild(bottomRightCorner, -1);
}

inline void fitKDLListTitle(GJListLayer* list, float maxWidth = 280.0f) {
    for (auto child : CCArrayExt<CCNode*>(list->getChildren())) {
        if (auto label = typeinfo_cast<CCLabelBMFont*>(child)) {
            label->limitLabelWidth(maxWidth, 1.0f, 0.2f);
            break;
        }
    }
}
