#pragma once
#include <Geode/Geode.hpp>
#include "../data/LevelEntry.hpp"
#include <memory>

namespace tulip::editor {
    class BrowserManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        BrowserManager();
        ~BrowserManager();

    public:

        static BrowserManager* get();

        void init();

        cocos2d::CCArray* getShadowMyLevels();
        cocos2d::CCArray* getLocalLevels(int folder);
        cocos2d::CCArray* getMyLevels();
        cocos2d::CCArray* getSharedLevels();
        cocos2d::CCArray* getDiscoverLevels();

        bool isMyLevel(std::string_view key);
        bool isMyLevel(GJGameLevel* level);
        bool isSharedLevel(GJGameLevel* level);
        bool isDiscoverLevel(GJGameLevel* level);

        std::optional<std::string> getLevelKey(GJGameLevel* level);
        LevelEntry* getLevelEntry(GJGameLevel* level);
        LevelEntry* getLevelEntry(std::string_view key);

        GJGameLevel* getShadowLevel(GJGameLevel* level);
        GJGameLevel* getReflectedLevel(GJGameLevel* level);
        bool isShadowLevel(GJGameLevel* level);

        void updateMyLevels(std::vector<LevelEntry>&& entries);
        void updateSharedLevels(std::vector<LevelEntry>&& entries);
        void updateDiscoverLevels(std::vector<LevelEntry>&& entries);

        void addLevelEntry(GJGameLevel* level, LevelEntry entry);
        void saveLevelEntry(LevelEntry const& entry);

        void setLevelValues(GJGameLevel* level, LevelEntry const& entry);

        void createShadowLevel(GJGameLevel* level);
        void removeShadowLevel(GJGameLevel* level);
    };
}