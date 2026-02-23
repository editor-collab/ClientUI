#include <hooks/ui/LevelEditorLayer.hpp>
#include <managers/LevelManager.hpp>
#include <ui/EditorOverlay.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>


using namespace geode::prelude;
using namespace tulip::editor;

void LevelEditorLayerUIHook::queueVisibility(bool visible) {
    Loader::get()->queueInMainThread([this, visible]() {
        static_cast<EditorUIUIHook*>(m_editorUI)->setVisibility(visible);
    });
}

void LevelEditorLayerUIHook::queueNotification(std::string message, geode::NotificationIcon icon, float duration) {
    Loader::get()->queueInMainThread([this, message = std::move(message), icon, duration]() {
        if (m_fields->notification) {
            m_fields->notification->hide();
        }
        m_fields->notification = Notification::create(message, icon, duration);
        m_fields->notification->show();
    });
}

bool LevelEditorLayerUIHook::init(GJGameLevel* level, bool p1) {
    if (!LevelEditorLayer::init(level, p1)) {
        return false;
    }

    LevelEntry* entry = BrowserManager::get()->getLevelEntry(LevelEditorLayer::get()->m_level);
	if (entry == nullptr) return true;

    auto old = m_objectLayer->getChildByType<EditorOverlay>(0);
    if (old == nullptr) {
        auto overlay = EditorOverlay::create(entry->hostAccountId);
        overlay->setID("selection-overlay"_spr);
        m_fields->editorOverlay = overlay;
		m_objectLayer->addChild(overlay, 10000);
    }

    m_fields->notification = Notification::create("Connecting to server, please wait...", NotificationIcon::Loading, 0.f);
    m_fields->notification->show();

    log::debug("Setting up socket event listeners");
    m_fields->socketConnectedHandle = Dispatch<>("alk.editor-collab/socket-connected").listen([this]() {
        log::info("Socket connected");
        this->queueVisibility(true);
        this->queueNotification("Connected to server!", NotificationIcon::Success, 2.f);
        LevelManager::get()->setSocketStatus(SocketStatus::Connected);
        return ListenerResult::Propagate;
    });
    m_fields->socketReconnectedHandle = Dispatch<>("alk.editor-collab/socket-reconnected").listen([this]() {
        log::info("Socket reconnected");
        this->queueVisibility(true);
        this->queueNotification("Reconnected to server!", NotificationIcon::Success, 2.f);
        LevelManager::get()->setSocketStatus(SocketStatus::Connected);
        return ListenerResult::Propagate;
    });
    m_fields->socketDisconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-disconnected").listen([this](std::string_view reason) {
        log::info("Socket disconnected: {}", reason);
        this->queueVisibility(false);
        LevelManager::get()->setSocketStatus(SocketStatus::Disconnected);
        return ListenerResult::Propagate;
    });
    m_fields->socketReconnectingHandle = Dispatch<int>("alk.editor-collab/socket-reconnecting").listen([this](int timer) {
        log::warn("Socket reconnecting: {}", timer);
        this->queueNotification(fmt::format("Connection failed, reconnecting in {}ms...", timer), NotificationIcon::Warning, 0.f);
        LevelManager::get()->setSocketStatus(SocketStatus::Reconnecting);
        return ListenerResult::Propagate;
    });
    m_fields->socketAbnormallyDisconnectedHandle = Dispatch<std::string_view>("alk.editor-collab/socket-abnormally-disconnected").listen([this](std::string_view reason) {
        log::warn("Socket abnormally disconnected: {}", reason);
        this->queueVisibility(false);
        LevelManager::get()->setSocketStatus(SocketStatus::Disconnected);
        return ListenerResult::Propagate;
    });

    return true;
}