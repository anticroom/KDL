#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
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
				CCSprite::create((Mod::get()->getResourcesDir() / "icon.png").string().c_str()),
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