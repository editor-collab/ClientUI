#pragma once
#include <Geode/Geode.hpp>
#include "../data/LevelEntry.hpp"
#include <memory>

namespace tulip::editor {
    class ShadowLevel : public GJGameLevel {
    public:
        static ShadowLevel* create() {
            auto ret =static_cast<ShadowLevel*>(GJGameLevel::create());
            // geode::log::debug("Creating ShadowLevel {}", ret);
            ret->m_levelType = GJLevelType::Editor;
            return ret;
        }
    };
    class ReflectedLevel : public GJGameLevel {
    public:
        static ReflectedLevel* create() {
            auto ret = static_cast<ReflectedLevel*>(GJGameLevel::create());
            // geode::log::debug("Creating ReflectedLevel {}", ret);
            ret->m_levelType = GJLevelType::Editor;
            return ret;
        }
    };

    class BrowserManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        BrowserManager();
        ~BrowserManager();

    public:

        static BrowserManager* get();

        void init();

        cocos2d::CCArray* getLocalLevels(int folder);
        cocos2d::CCArray* getMyLevels();
        cocos2d::CCArray* getSharedLevels();
        cocos2d::CCArray* getDiscoverLevels();

        bool isMyLevel(GJGameLevel* level);
        bool isSharedLevel(GJGameLevel* level);
        bool isDiscoverLevel(GJGameLevel* level);
        bool isOnlineLevel(GJGameLevel* level);

        bool hasLevelEntry(GJGameLevel* level);
        void addLevelEntry(GJGameLevel* level, LevelEntry&& entry);
        void updateLevelEntry(GJGameLevel* level);
        LevelEntry* getLevelEntry(GJGameLevel* level);

        bool hasOnlineEntry(GJGameLevel* level);
        LevelEntry* getOnlineEntry(GJGameLevel* level);

        void updateMyLevels(std::vector<LevelEntry>&& entries);
        void updateSharedLevels(std::vector<LevelEntry>&& entries);
        void updateDiscoverLevels(std::vector<LevelEntry>&& entries);

        void setLevelValues(GJGameLevel* level, LevelEntry const& entry);
        void saveLevel(GJGameLevel* level, bool insert = false, bool override = false);

        void replaceWithShadowLevel(GJGameLevel*& level, bool create = false);
        void detachReflectedLevel(GJGameLevel*& level);

        void initializeKey(GJGameLevel* level, LevelEntry const& entry);
    };
}