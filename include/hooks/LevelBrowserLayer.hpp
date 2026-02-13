#pragma once

#include <Geode/Geode.hpp>  
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct LevelBrowserLayerHook : Modify<LevelBrowserLayerHook, LevelBrowserLayer> {
        struct Fields {
            geode::Ref<CCMenuItemSpriteExtra> menuButton = nullptr;
            geode::async::TaskHolder<bool> modPageTask;

            geode::async::TaskHolder<web::WebResponse> adminTask;
            geode::async::TaskHolder<Result<std::string>> loginListener;
            static inline LevelBrowserLayerHook* self = nullptr;

            ~ Fields() {
                self = nullptr;
            }
        };

        void refreshButton();

        void onLogin(Result<std::string> result);
        void onLogout(Result<> result);

        $override
        bool init(GJSearchObject* searchObject);

        LevelBrowserLayerHook* from(LevelBrowserLayer* layer) {
            return static_cast<LevelBrowserLayerHook*>(layer);
        }
    };
}