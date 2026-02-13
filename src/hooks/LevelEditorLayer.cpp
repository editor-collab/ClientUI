#include <hooks/LevelEditorLayer.hpp>
#include <managers/LevelManager.hpp>
#include <managers/WebManager.hpp>
#include <ui/EditorOverlay.hpp>
#include <managers/BrowserManager.hpp>
#include <Geode/loader/Dispatch.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool LevelEditorLayerHook::init(GJGameLevel* level, bool p1) {
    if (!LevelEditorLayer::init(level, p1)) {
        return false;
    }

    LevelEntry* entry = BrowserManager::get()->getLevelEntry(LevelEditorLayer::get()->m_level);
	if (entry == nullptr) return true;

    m_fields->levelKickedHandle = Dispatch<std::string_view>("alk.editor-collab/level-kicked").listen([this](std::string_view reason) {
        log::debug("Level kicked: {}", reason);
        LevelManager::get()->leaveLevelAbnormal();
        return ListenerResult::Propagate;
    });
    m_fields->updateSnapshotHandle = Dispatch<std::string_view>("alk.editor-collab/update-level-snapshot").listen([this](std::string_view token) {
        std::vector<uint8_t> snapshot;
        Dispatch<std::vector<uint8_t>*>("get-level-snapshot"_spr).send(&snapshot);
        if (snapshot.empty()) {
            log::warn("No snapshot data available for update");
            return ListenerResult::Propagate;
        }
        m_fields->updateSnapshotTask.spawn(
            LevelManager::get()->updateLevelSnapshot(LevelManager::get()->getJoinedLevelKey().value(), token, snapshot),
            []() {}
        );
        return ListenerResult::Propagate;
    });

    // m_fields->socketConnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-connected").listen([this](std::string_view reason) {
    //     log::debug("Socket connected: {}", reason);
    //     WebManager::get()->connectSocket();
    //     return ListenerResult::Propagate;
    // });
    // m_fields->socketDisconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-disconnected").listen([this](std::string_view reason) {
    //     log::debug("Socket disconnected: {}", reason);
    //     WebManager::get()->disconnectSocket();
    //     return ListenerResult::Propagate;
    // });
    // m_fields->socketAbnormallyDisconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-abnormally-disconnected").listen([this](std::string_view reason) {
    //     log::debug("Socket abnormally disconnected: {}", reason);
    //     WebManager::get()->disconnectSocket();
    //     return ListenerResult::Propagate;
    // });

    return true;
}