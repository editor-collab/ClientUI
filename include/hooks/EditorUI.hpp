#pragma once

#include <Geode/Geode.hpp>

#include <Geode/modify/EditorUI.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct EditorUIHook : Modify<EditorUIHook, EditorUI> {
        struct Fields {
            geode::Ref<geode::EventListenerNode<geode::DispatchFilter<std::string_view>>> kickListener;
        };

        $override
        bool init(LevelEditorLayer* editorLayer);
    };
}