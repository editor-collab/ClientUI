#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "../../managers/FetchManager.hpp"

#include <Geode/modify/LevelBrowserLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    enum class CurrentTab {
        LocalLevels,
        MyLevels,
        SharedWithMe,
        Discover,
    };

    struct LevelBrowserLayerUIHook : Modify<LevelBrowserLayerUIHook, LevelBrowserLayer> {
        struct Fields {
            CurrentTab currentTab = CurrentTab::LocalLevels;

            geode::ListView* m_myLevelsList;

            std::vector<geode::Ref<GJGameLevel>> missingMyLevels;

            geode::async::TaskHolder<geode::Result<std::vector<LevelEntry>>> myLevelsListener;
            geode::async::TaskHolder<geode::Result<std::vector<LevelEntry>>> sharedWithMeListener;
            geode::async::TaskHolder<geode::Result<std::vector<LevelEntry>>> discoverListener;

            geode::Ref<cocos2d::CCMenu> offTabMenu;
            geode::Ref<cocos2d::CCMenu> onTabMenu;

            static inline bool initialCall = false;
        };

        template <class Lambda>
        CCMenuItemSpriteExtra* generateTabButton(std::string_view framename, std::string_view id, Lambda&& func);

        CCSprite* generateTabSprite(std::string_view framename, std::string_view id, bool visible);

        void revisualizeButtons(cocos2d::CCObject* sender);

        $override
        void setupLevelBrowser(cocos2d::CCArray* items);

        $override
        void loadLevelsFinished(CCArray* levels, char const* ident, int searchType);

        $override
        bool init(GJSearchObject* searchObject);

        void onLocalLevels(cocos2d::CCObject* sender);
        void onMyLevels(cocos2d::CCObject* sender);
        void onSharedWithMe(cocos2d::CCObject* sender);
        void onDiscover(cocos2d::CCObject* sender);

        static LevelBrowserLayerUIHook* from(LevelBrowserLayer* layer) {
            return static_cast<LevelBrowserLayerUIHook*>(layer);
        }
    };
}