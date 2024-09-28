#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "../../managers/FetchManager.hpp"

#include <Geode/modify/LevelBrowserLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    enum class CurrentTab {
        MyLevels,
        SharedWithMe,
        Discover,
    };

    struct LevelBrowserLayerUIHook : Modify<LevelBrowserLayerUIHook, LevelBrowserLayer> {
        struct Fields {
            CurrentTab currentTab = CurrentTab::MyLevels;

            geode::ListView* m_myLevelsList;

            std::vector<geode::Ref<GJGameLevel>> missingMyLevels;

            EventListener<FetchManager::TaskType> myLevelsListener;
            EventListener<FetchManager::TaskType> sharedWithMeListener;
            EventListener<FetchManager::TaskType> discoverListener;
        };

        template <class Lambda>
        CCMenuItemSpriteExtra* generateTabButton(std::string_view framename, std::string_view id, Lambda&& func);

        CCSprite* generateTabSprite(std::string_view framename, std::string_view id, bool visible);

        void revisualizeButtons(CCMenu* tabMenu, CCMenu* tabMenu2, CCNode* sender);

        $override
        void setupLevelBrowser(cocos2d::CCArray* items);

        $override
        void loadLevelsFinished(CCArray* levels, char const* ident, int searchType);

        $override
        bool init(GJSearchObject* searchObject);

        LevelBrowserLayerUIHook* from(LevelBrowserLayer* layer) {
            return static_cast<LevelBrowserLayerUIHook*>(layer);
        }
    };
}