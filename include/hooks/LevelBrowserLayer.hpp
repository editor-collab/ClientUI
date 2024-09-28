#pragma once

#include <Geode/Geode.hpp>  
#include <Geode/utils/web.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct LevelBrowserLayerHook : Modify<LevelBrowserLayerHook, LevelBrowserLayer> {
        struct Fields {
            EventListener<Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, web::WebProgress>> joinLevelListener;
        };

        void onChallenge(Result<> result);
        void onLogin(Result<> result, bool challenge = false);
        void onConnect();

        $override
        bool init(GJSearchObject* searchObject);

        LevelBrowserLayerHook* from(LevelBrowserLayer* layer) {
            return static_cast<LevelBrowserLayerHook*>(layer);
        }
    };
}