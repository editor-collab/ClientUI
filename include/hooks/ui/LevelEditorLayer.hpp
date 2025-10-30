
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelEditorLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    class EditorOverlay;
    struct LevelEditorLayerUIHook : Modify<LevelEditorLayerUIHook, LevelEditorLayer> {
        struct Fields {
            EditorOverlay* editorOverlay = nullptr;
        };

        $override
        bool init(GJGameLevel* level, bool p1);
    };
}