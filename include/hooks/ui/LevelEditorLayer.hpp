
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelEditorLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    class EditorOverlay;
    struct LevelEditorLayerUIHook : Modify<LevelEditorLayerUIHook, LevelEditorLayer> {
        struct Fields {
            EditorOverlay* editorOverlay = nullptr;

            geode::Ref<geode::Notification> notification;

            ListenerHandle actionsLoadedHandle;
            ListenerHandle socketConnectedHandle;
            ListenerHandle socketReconnectedHandle;
            ListenerHandle socketDisconnectedHandle;
            ListenerHandle socketReconnectingHandle;
            ListenerHandle socketAbnormallyDisconnectedHandle;

            ~Fields() {
                if (notification) {
                    notification->cancel();
                    notification = nullptr;
                }
            }
        };

        $override
        bool init(GJGameLevel* level, bool p1);

        void queueVisibility(bool visible);

        void queueNotification(std::string message, geode::NotificationIcon icon, float duration);
    };
}