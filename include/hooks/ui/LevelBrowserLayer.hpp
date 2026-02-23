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
            static inline CurrentTab currentTab = CurrentTab::LocalLevels;

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

        void revisualizeButtons();

        $override
        void setupLevelBrowser(cocos2d::CCArray* items);

        $override
        void loadLevelsFinished(CCArray* levels, char const* ident, int searchType);

        $override
        bool init(GJSearchObject* searchObject);

        void onLocalLevels();
        void onMyLevels();
        void onSharedWithMe();
        void onDiscover();

        static LevelBrowserLayerUIHook* from(LevelBrowserLayer* layer) {
            return static_cast<LevelBrowserLayerUIHook*>(layer);
        }
    };
}