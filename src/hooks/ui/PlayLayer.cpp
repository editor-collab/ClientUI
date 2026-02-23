#include <hooks/ui/PlayLayer.hpp>
#include <managers/LevelManager.hpp>
#include <ui/EditorOverlay.hpp>
#include <managers/BrowserManager.hpp>


using namespace geode::prelude;
using namespace tulip::editor;

bool PlayLayerUIHook::init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
        return false;
    }

    LevelEntry* entry = BrowserManager::get()->getLevelEntry(PlayLayer::get()->m_level);
	if (entry == nullptr) return true;

    m_fields->socketConnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-connected").listen([this](std::string_view reason) {
        log::debug("Socket reconnected: {}", reason);
        LevelManager::get()->setSocketStatus(SocketStatus::Connected);
        return ListenerResult::Propagate;
    });
    m_fields->socketReconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-reconnected").listen([this](std::string_view reason) {
        log::debug("Socket reconnected: {}", reason);
        LevelManager::get()->setSocketStatus(SocketStatus::Connected);
        return ListenerResult::Propagate;
    });
    m_fields->socketDisconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-disconnected").listen([this](std::string_view reason) {
        log::debug("Socket disconnected: {}", reason);
        LevelManager::get()->setSocketStatus(SocketStatus::Disconnected);
        return ListenerResult::Propagate;
    });
    m_fields->socketAbnormallyDisconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-abnormally-disconnected").listen([this](std::string_view reason) {
        log::debug("Socket abnormally disconnected: {}", reason);
        LevelManager::get()->setSocketStatus(SocketStatus::Reconnecting);
        return ListenerResult::Propagate;
    });

    return true;
}