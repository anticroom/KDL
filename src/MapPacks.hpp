#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class MapPacks : public CCLayer, public LevelManagerDelegate {
protected:
    std::vector<int> m_allIds;
    GJListLayer* m_listLayer = nullptr;
    CCMenu* m_tabMenu = nullptr;

    bool init() {
        if (!CCLayer::init()) return false;
		auto winSize = CCDirector::get()->getWinSize();
        this->setID("kdl-pack-layer");
        this->setKeyboardEnabled(true);
        this->setKeypadEnabled(true);
		
        // background
		auto background = CCSprite::create("GJ_gradientBG.png");
        background->setID("background");
        background->setScaleX(winSize.width / background->getContentSize().width);
        background->setScaleY(winSize.height / background->getContentSize().height);
        background->setPosition(winSize / 2);
        background->setColor({40, 0, 80});
        this->addChild(background, -1);

        // back arrow
		auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto backButton = CCMenuItemSpriteExtra::create(
            backSprite, this, menu_selector(MapPacks::onBack)
        );
        backButton->setID("back-button");

        // top "stuff"
		auto topMenu = CCMenu::create();
        topMenu->setID("top-menu");
        topMenu->setPosition({0.0f, 0.0f});
        backButton->setPosition({25.0f, winSize.height - 25.0f});
        topMenu->addChild(backButton);
        this->addChild(topMenu);

        // not so top "stuff"  
        m_tabMenu = CCMenu::create();
        m_tabMenu->setID("tab-menu");
        m_tabMenu->setPosition({0.0f, 0.0f});
        m_tabMenu->setContentSize({winSize.width, 50.0f});


        // map packs
        auto packsArr = makePacks();

        auto listView = CustomListView::create(packsArr, BoomListType::MapPack, 220.0f, 356.0f);

        m_listLayer = GJListLayer::create(listView, "KDL", {0, 0, 0, 180}, 356.0f, 220.0f, 0);
        m_listLayer->setID("list-layer");
        m_listLayer->setPosition(winSize / 2 - m_listLayer->getScaledContentSize() / 2);
        this->addChild(m_listLayer, 1);
        this->addChild(m_tabMenu, 2);

        return true;
	}

public:
    static MapPacks* create() {
        auto ret = new MapPacks();
        if (ret->init()) { 
            ret->autorelease();
            return ret;
        }
        
        delete ret;
        return nullptr;
    }

    static CCScene* scene() {
        auto s = CCScene::create();
        s->addChild(MapPacks::create());
        return s;
    }

    void loadLevelsFinished(CCArray* levels, const char*) override {}

    void loadLevelsFailed(const char*) override {}

    void onExit() override {
        auto glm = GameLevelManager::sharedState();
        if (glm->m_levelManagerDelegate == this)
            glm->m_levelManagerDelegate = nullptr;
        CCLayer::onExit();
    }

    CCArray* makePacks() {
        auto packsArr = CCArray::create();

        auto res = web::WebRequest().getSync("https://therealkeanan00s-demon-list.com/api/map-packs");
        if (!res.ok()) return CCArray::create();

        auto jsonRes = res.json();
        if (!jsonRes) return CCArray::create();

        auto json = jsonRes.unwrap()["packs"];
        auto arr = json.asArray();
        if (!arr) return CCArray::create();

        auto entries = arr.unwrap();
        auto ids = 1000;

        for (auto& entry : entries) {
            auto pack = GJMapPack::create();
            pack->m_packID = ids;
            pack->m_packName = entry.get<std::string>("name").unwrapOr("Unknown");
            auto levelIds = CCArray::create();
            auto lvls = entry.get<std::vector<int>>("levels").unwrapOr(std::vector<int>{});
            for (auto& lvl : lvls) {
                levelIds->addObject(CCString::create(std::to_string(lvl)));
            }
            levelIds->retain();
            pack->m_levels = levelIds;
            pack->m_difficulty = static_cast<GJDifficulty>(entry.get<int>("difficulty").unwrapOr(-1));
            pack->m_stars = -10;
            pack->m_coins = -10;
            auto textColor = entry.get<std::vector<int>>("tc").unwrapOr(std::vector<int>{255,255,255});
            auto barColor = entry.get<std::vector<int>>("bc").unwrapOr(std::vector<int>{255,255,255});
            pack->m_textColour = ccc3(textColor[0], textColor[1], textColor[2]);
            pack->m_barColour = ccc3(barColor[0], barColor[1], barColor[2]);
            packsArr->addObject(pack);
            ids += 1;
        }
        return packsArr;
    }

    void onEnter() override {
        CCLayer::onEnter();
        GameLevelManager::sharedState()->m_levelManagerDelegate = this;
    }

	void onBack(CCObject*) {
        CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    }

    void keyBackClicked() override {
        CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    }
};