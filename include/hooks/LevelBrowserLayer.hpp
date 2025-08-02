#pragma once

#include <Geode/Geode.hpp>  
#include <Geode/utils/web.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct LevelBrowserLayerHook : Modify<LevelBrowserLayerHook, LevelBrowserLayer> {
        struct Fields {
            geode::Ref<CCMenuItemSpriteExtra> menuButton = nullptr;
            geode::Task<bool> modPageTask;

            static inline LevelBrowserLayerHook* self = nullptr;

            ~ Fields() {
                self = nullptr;
            }
        };

        void refreshButton();

        void onLogin(Result<> result);
        void onLogout(Result<> result);

        $override
        bool init(GJSearchObject* searchObject);

        LevelBrowserLayerHook* from(LevelBrowserLayer* layer) {
            return static_cast<LevelBrowserLayerHook*>(layer);
        }
    };
}