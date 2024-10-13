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

        cocos2d::CCArray* getMyLevels(int folder);
        cocos2d::CCArray* getSharedLevels();
        cocos2d::CCArray* getDiscoverLevels();

        bool isMyLevel(GJGameLevel* level);
        bool isSharedLevel(GJGameLevel* level);
        bool isDiscoverLevel(GJGameLevel* level);

        std::optional<std::string> getLevelKey(GJGameLevel* level);
        std::optional<LevelEntry*> getLevelEntry(GJGameLevel* level);

        void updateMyLevels(std::vector<LevelEntry>&& entries);
        void updateSharedLevels(std::vector<LevelEntry>&& entries);
        void updateDiscoverLevels(std::vector<LevelEntry>&& entries);
    };
}