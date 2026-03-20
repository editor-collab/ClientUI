#include <hooks/ui/LevelEditorLayer.hpp>
#include <managers/LevelManager.hpp>
#include <ui/EditorOverlay.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>


using namespace geode::prelude;
using namespace tulip::editor;

void LevelEditorLayerUIHook::queueVisibility(bool visible) {
    Loader::get()->queueInMainThread([visible]() {
        if (auto editorUI = EditorUI::get()) {
            static_cast<EditorUIUIHook*>(editorUI)->setVisibility(visible);
        }
    });
}

void LevelEditorLayerUIHook::queueNotification(std::string message, geode::NotificationIcon icon, float duration) {
    Loader::get()->queueInMainThread([message = std::move(message), icon, duration]() {
        if (auto editorLayer = LevelEditorLayer::get()) {
            if (static_cast<LevelEditorLayerUIHook*>(editorLayer)->m_fields->notification) {
                static_cast<LevelEditorLayerUIHook*>(editorLayer)->m_fields->notification->cancel();
            }
            static_cast<LevelEditorLayerUIHook*>(editorLayer)->m_fields->notification = Notification::create(message, icon, duration);
            static_cast<LevelEditorLayerUIHook*>(editorLayer)->m_fields->notification->show();
        }
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

    Loader::get()->queueInMainThread([this]() {
        if (static_cast<EditorUIUIHook*>(m_editorUI)->m_fields->visibility) {
            return;
        }
        if (m_fields->notification) {
            m_fields->notification->cancel();
        }
        m_fields->notification = Notification::create("Connecting to server, please wait...", NotificationIcon::Loading, 0.f);
        m_fields->notification->show();
    });

    log::debug("Setting up socket event listeners");
    m_fields->actionsLoadedHandle = Dispatch<>("alk.editor-collab/actions-loaded").listen([this]() {
        log::info("Actions loaded");
        this->queueVisibility(true);
        this->queueNotification("Loaded actions!", NotificationIcon::Success, 2.f);
        LevelManager::get()->setSocketStatus(SocketStatus::Connected);
        return ListenerResult::Propagate;
    });
    m_fields->socketConnectedHandle = Dispatch<>("alk.editor-collab/socket-connected").listen([this]() {
        log::info("Socket connected");
        // this->queueVisibility(true);
        this->queueNotification("Connected to server, loading actions...", NotificationIcon::Loading, 0.f);
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

    m_fields->levelKickedHandle = Dispatch<std::string_view>("alk.editor-collab/level-kicked").listen([=, this](std::string_view reason) {
        log::debug("Level kicked: {}", reason);
        Loader::get()->queueInMainThread([=]() {
            Notification::create(std::string(reason), NotificationIcon::Info)->show();
            GameManager::get()->returnToLastScene(level);
            LevelManager::get()->leaveLevelAbnormal();
        });
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
            LevelManager::get()->updateLevelSnapshot(LevelManager::get()->getJoinedLevelKey(), std::string(token), std::move(snapshot)),
            [](auto res) {}
        );
        return ListenerResult::Propagate;
    });

    return true;
}