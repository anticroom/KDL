#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <thread>
#include "KDLBackground.hpp"

using namespace geode::prelude;

struct KDLLeaderboardEntry {
    int rank;
    std::string name;
    int points;
};

class KDLLeaderboardCell : public CCLayer {
public:
    static constexpr float CELL_WIDTH = 356.0f;
    static constexpr float CELL_HEIGHT = 35.0f;

    static KDLLeaderboardCell* create(KDLLeaderboardEntry const& entry, bool creator, int index) {
        auto ret = new KDLLeaderboardCell();
        if (ret->init(entry, creator, index)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

protected:
    bool init(KDLLeaderboardEntry const& entry, bool creator, int index) {
        if (!CCLayer::init()) return false;

        setContentSize({CELL_WIDTH, CELL_HEIGHT});

        auto bg = CCLayerColor::create(ccc4(0, 0, 0, index % 2 == 0 ? 95 : 55), CELL_WIDTH, CELL_HEIGHT);
        bg->setPosition({0.0f, 0.0f});
        addChild(bg);

        auto rankLabel = CCLabelBMFont::create(fmt::format("#{}", entry.rank).c_str(), "bigFont.fnt");
        if (entry.rank == 1) rankLabel->setColor({255, 200, 50});
        else if (entry.rank == 2) rankLabel->setColor({200, 200, 200});
        else if (entry.rank == 3) rankLabel->setColor({210, 140, 70});
        rankLabel->setPosition({25.0f, CELL_HEIGHT / 2.0f});
        rankLabel->setScale(entry.rank <= 3 ? 0.60f : 0.45f);
        rankLabel->setID("rank-label");
        addChild(rankLabel);

        auto nameLabel = CCLabelBMFont::create(entry.name.c_str(), "bigFont.fnt");
        nameLabel->setAnchorPoint({0.0f, 0.5f});
        nameLabel->setPosition({55.0f, CELL_HEIGHT / 2.0f});
        nameLabel->limitLabelWidth(150.0f, 0.55f, 0.0f);
        nameLabel->setID("name-label");
        addChild(nameLabel);

        auto ptsLabel = CCLabelBMFont::create(fmt::format("{} pts", entry.points).c_str(), "goldFont.fnt");
        ptsLabel->setAnchorPoint({1.0f, 0.5f});
        ptsLabel->setPosition({CELL_WIDTH - 36.0f, CELL_HEIGHT / 2.0f});
        ptsLabel->setScale(0.45f);
        ptsLabel->setID("points-label");
        addChild(ptsLabel);

        CCNode* icon = nullptr;
        float targetHeight = 13.5f;
        if (auto smallFrame = CCSpriteFrameCache::get()->spriteFrameByName("difficulty_10_btn_001.png")) {
            targetHeight = smallFrame->getOriginalSize().height * 0.32f;
        }

        if (creator) {
            icon = CCSprite::createWithSpriteFrameName("GJ_hammerIcon_001.png");
        } else {
            icon = CCSprite::create("KDL_diffIcon_10_btn_001.png"_spr);
            if (!icon) icon = CCSprite::createWithSpriteFrameName("difficulty_10_btn_001.png");
        }

        if (icon) {
            icon->setScale(targetHeight / icon->getContentHeight());
            icon->setPosition({CELL_WIDTH - 20.0f, CELL_HEIGHT / 2.0f});
            icon->setID("points-icon");
            addChild(icon);
        }

        return true;
    }
};

class LeaderboardLayer : public CCLayer {
protected:
    GJListLayer* m_list = nullptr;
    LoadingCircle* m_loadingCircle = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    int m_requestId = 0;

    virtual const char* title() const { return "K.D.L. Leaderboard"; }
    virtual const char* url() const { return "https://the-kdl.com/api/lleaderboard"; }
    virtual bool isCreator() const { return false; }
    virtual ccColor3B bgColor() const { return {0, 40, 80}; }

    bool init() override {
        if (!CCLayer::init()) return false;

        auto winSize = CCDirector::get()->getWinSize();
        this->setKeyboardEnabled(true);
        this->setKeypadEnabled(true);

        addKDLBackground(this, bgColor());

        m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_countLabel->setAnchorPoint({1.0f, 1.0f});
        m_countLabel->setScale(0.6f);
        m_countLabel->setPosition({winSize.width - 7.0f, winSize.height - 3.0f});
        m_countLabel->setID("count-label");
        this->addChild(m_countLabel, 2);

        auto menu = CCMenu::create();
        menu->setPosition({0.0f, 0.0f});
        menu->setID("button-menu");
        this->addChild(menu, 3);

        auto backButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
            this, menu_selector(LeaderboardLayer::onBack)
        );
        backButton->setPosition({25.0f, winSize.height - 25.0f});
        backButton->setID("back-button");
        menu->addChild(backButton);

        auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        auto refreshButton = CCMenuItemSpriteExtra::create(
            refreshBtnSpr, this, menu_selector(LeaderboardLayer::onRefresh)
        );
        refreshButton->setPosition({
            winSize.width - refreshBtnSpr->getContentWidth() / 2.0f - 4.0f,
            refreshBtnSpr->getContentHeight() / 2.0f + 4.0f
        });
        refreshButton->setID("refresh-button");
        menu->addChild(refreshButton);

        m_list = GJListLayer::create(nullptr, title(), {0, 0, 0, 180}, 356.0f, 220.0f, 0);
        m_list->setPosition(winSize / 2.0f - m_list->getContentSize() / 2.0f);
        m_list->setID("list-layer");
        fitKDLListTitle(m_list);
        this->addChild(m_list, 3);

        m_loadingCircle = LoadingCircle::create();
        m_loadingCircle->setParentLayer(this);
        m_loadingCircle->setID("loading-circle");
        m_loadingCircle->show();

        loadLeaderboard();

        return true;
    }

    void loadLeaderboard() {
        m_loadingCircle->setVisible(true);

        int requestId = ++m_requestId;
        std::string requestUrl = url();
        Ref<LeaderboardLayer> self = this;

        std::thread([self, requestId, requestUrl] {
            auto res = web::WebRequest().getSync(requestUrl);

            std::vector<KDLLeaderboardEntry> entries;
            int errorCode = 0;

            if (!res.ok()) {
                errorCode = res.code();
            } else {
                auto jsonRes = res.json();
                if (!jsonRes.isOk()) {
                    errorCode = 500;
                } else {
                    auto arr = jsonRes.unwrap().asArray();
                    if (!arr.isOk()) {
                        errorCode = 500;
                    } else {
                        int rank = 1;
                        for (auto& entry : arr.unwrap()) {
                            entries.push_back({
                                rank++,
                                entry.get<std::string>("name").unwrapOr("Unknown"),
                                entry.get<int>("points").unwrapOr(0)
                            });
                        }
                    }
                }
            }

            Loader::get()->queueInMainThread([self, requestId, errorCode, entries = std::move(entries)] {
                // ignore stale responses from stuff like refresh spamming
                if (requestId != self->m_requestId) return;
                if (errorCode != 0) self->onLoadFailed(errorCode);
                else self->populateList(entries);
            });
        }).detach();
    }

    void populateList(std::vector<KDLLeaderboardEntry> const& entries) {
        if (auto listView = m_list->m_listView) {
            listView->removeFromParent();
            listView->release();
            m_list->m_listView = nullptr;
        }

        auto cells = CCArray::create();
        int index = 0;
        for (auto& entry : entries) {
            cells->addObject(KDLLeaderboardCell::create(entry, isCreator(), index++));
        }

        auto listView = ListView::create(cells, KDLLeaderboardCell::CELL_HEIGHT, 356.0f, 220.0f);
        listView->retain();
        m_list->addChild(listView, 6, 9);
        m_list->m_listView = listView;

        m_loadingCircle->setVisible(false);
    }

    void onLoadFailed(int code) {
        m_loadingCircle->setVisible(false);
        FLAlertLayer::create(
            fmt::format("Load Failed ({})", code).c_str(),
            "Failed to load leaderboard! Please try again later.. Please Message Keanan about this issue if it persists.",
            "OK"
        )->show();
    }

    void onRefresh(CCObject*) {
        loadLeaderboard();
    }

    void onBack(CCObject*) {
        keyBackClicked();
    }

    void keyBackClicked() override {
        CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    }

public:
    static LeaderboardLayer* create() {
        auto ret = new LeaderboardLayer();

        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    static CCScene* scene() {
        auto s = CCScene::create();
        s->addChild(LeaderboardLayer::create());
        return s;
    }
};
