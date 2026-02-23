
#include <Geode/Geode.hpp>

#include <Geode/modify/PlayLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    class EditorOverlay;
    struct PlayLayerUIHook : Modify<PlayLayerUIHook, PlayLayer> {
        struct Fields {
            ListenerHandle socketConnectedHandle;
            ListenerHandle socketReonnectedHandle;
            ListenerHandle socketDisconnectedHandle;
            ListenerHandle socketAbnormallyDisconnectedHandle;
        };

        $override
        bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects);
    };
}