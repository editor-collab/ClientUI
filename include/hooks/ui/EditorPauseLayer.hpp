#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "../../managers/FetchManager.hpp"

#include <Geode/modify/EditorPauseLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct EditorPauseLayerUIHook : Modify<EditorPauseLayerUIHook, EditorPauseLayer> {
        $override
        bool init(LevelEditorLayer* editorLayer);
        
        void setupGuidelinesMenu();
        void setupInfoMenu();
    };
}