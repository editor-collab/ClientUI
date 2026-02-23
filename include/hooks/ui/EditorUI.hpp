#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "../../managers/FetchManager.hpp"

#include <Geode/modify/EditorUI.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct EditorUIUIHook : Modify<EditorUIUIHook, EditorUI> {
        struct Fields {
            CCMenuItemSpriteExtra* m_shareButton;
            EditButtonBar* savedButtonBar = nullptr;
            bool visibility = true;
            ListenerHandle uiVisibilityHandle;
        };

        void setVisibility(bool show);

        $override
        bool init(LevelEditorLayer* editorLayer);

        $override
        void showUI(bool show);
    };
}