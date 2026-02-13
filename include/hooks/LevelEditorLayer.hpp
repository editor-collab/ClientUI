
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelEditorLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct LevelEditorLayerHook : Modify<LevelEditorLayerHook, LevelEditorLayer> {
        struct Fields {
            ListenerHandle levelKickedHandle;
            ListenerHandle updateSnapshotHandle;
            async::TaskHolder<Result<>> updateSnapshotTask;

            ListenerHandle socketConnectedHandle;
            ListenerHandle socketDisconnectedHandle;
            ListenerHandle socketAbnormallyDisconnectedHandle;
        };

        $override
        bool init(GJGameLevel* level, bool p1);
    };
}