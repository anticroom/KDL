#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/MapPackCell.hpp>
#include "KDLListLayer.hpp"
#include "MapPacks.hpp"

class $modify(MyCreatorLayer, CreatorLayer) {
	bool init() {
		if (!CreatorLayer::init()) {
			return false;
		}

		auto KDLButton = CCMenuItemSpriteExtra::create(
            CCSprite::create((Mod::get()->getResourcesDir() / "icon.png").string().c_str()),
			this,
			menu_selector(MyCreatorLayer::onKDLButton)
		);

		auto winSize = CCDirector::get()->getWinSize();
        auto listMenu = CCMenu::create();

		listMenu->addChild(KDLButton);
		KDLButton->setID("kdl-Button"_spr);
		listMenu->updateLayout();
		if (this->getChildByID("bottom-left-menu")) {
			auto menu = this->getChildByID("bottom-left-menu");
            menu->addChild(KDLButton);
        	menu->updateLayout();
        }

		return true;
	}

	void onKDLButton(CCObject*) {
		CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, KDLListLayer::scene()));
	}

	
};

class $modify(MyLevelBrowserLayer, LevelBrowserLayer) {
		bool init(GJSearchObject* object) {
			if (!LevelBrowserLayer::init(object)) return false;

			if (object->m_searchType != SearchType::MapPack) return true;

			auto winSize = CCDirector::get()->getWinSize();
			auto demonlistButtonSprite = CCMenuItemSpriteExtra::create(
				CCSprite::create((Mod::get()->getResourcesDir() / "mp.png").string().c_str()),
				this,
				menu_selector(MyLevelBrowserLayer::onKDLButton)
			);

			demonlistButtonSprite->setScale(1.0f);
			auto demonlistButton = CCMenuItemSpriteExtra::create(demonlistButtonSprite, this, menu_selector(MyLevelBrowserLayer::onKDLButton));
			demonlistButton->setID("demonlist-button"_spr);
			auto menu = CCMenu::create();
			menu->addChild(demonlistButton);
			menu->setPosition({winSize.width - demonlistButtonSprite->getContentHeight() / 2.0f - 4.0f, demonlistButtonSprite->getContentHeight() / 2.0f + 54.0f});
			menu->setID("demonlist-menu"_spr);
			addChild(menu, 2);

			return true;
    }

    void onKDLButton(CCObject* sender) {
		CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, MapPacks::scene()));
    }
};


constexpr int KDL_PACK_ID_MIN = 1000;

class $modify(KDLMapPackCell, MapPackCell) {
    void onClick(CCObject* sender) {
        if (m_mapPack && m_mapPack->m_packID >= KDL_PACK_ID_MIN) {
            std::string ids;
            if (m_mapPack->m_levels) {
                for (auto* obj : CCArrayExt<CCString*>(m_mapPack->m_levels)) {
                    if (!obj) continue;
                    if (!ids.empty()) ids += ",";
                    ids += obj->getCString();
                }
            }

            auto search = GJSearchObject::create(SearchType::Type19, ids);
            auto scene = LevelBrowserLayer::scene(search);
            CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, scene));
            return;
        }

        MapPackCell::onClick(sender);
    }
};

struct KdlEntry {
    int demonType;
    std::string rate;
};

static std::unordered_map<int, KdlEntry> g_kdlLevels;

$on_mod(Loaded) {
    std::thread([] {
        auto res = web::WebRequest().getSync("https://therealkeanan00s-demon-list.com/api/list/all");
        if (!res.ok()) return;

        auto jsonRes = res.json();
        if (!jsonRes) return;

        auto json = jsonRes.unwrap()["levels"];
        auto arr = json.asArray();
        if (!arr) return;

        for (auto& entry : arr.unwrap()) {
            auto id = entry.get<int>("id").unwrapOr(-1);
            auto type = entry.get<int>("demon_type").unwrapOr(1);
            auto rate = entry.get<std::string>("rate").unwrapOr("");
            if (id > 0) g_kdlLevels[id] = { type, rate };
        }
    }).detach();
}

static void rate(GJGameLevel* level) {
    auto it = g_kdlLevels.find(level->m_levelID);
    if (it == g_kdlLevels.end()) return;
    
    auto& entry = it->second;
    
    level->m_difficulty = GJDifficulty::Demon;
    level->m_demon = 1;
    level->m_demonDifficulty = entry.demonType;
    
    if (entry.rate == "featured") {
        level->m_featured = 1;
    } else if (entry.rate == "epic") {
        level->m_isEpic = 1;
    } else if (entry.rate == "legendary") {
        level->m_isEpic = 2;
    } else if (entry.rate == "mythic") {
        level->m_isEpic = 3;
    }
}

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        rate(level);
        if (!LevelInfoLayer::init(level, challenge)) return false;
        return true;
    }
};

class $modify(MyLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        rate(level);
        LevelCell::loadFromLevel(level);
    }
};