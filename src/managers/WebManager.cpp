#include <managers/WebManager.hpp>
#include <managers/AccountManager.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <chrono>

using namespace tulip::editor;
using namespace geode::prelude;

class WebManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<std::function<void()>> m_onSocketConnected;
    std::vector<std::function<void()>> m_onSocketDisconnected;
    bool m_socketConnected = false;

    void init();

    web::WebRequest createRequest() const;
    web::WebRequest createAuthenticatedRequest() const;
    std::string getServerURL() const;

    bool isSocketConnected() const;
    void runOnSocketConnected(std::function<void()>&& callback);
    void runOnSocketDisconnected(std::function<void()>&& callback);
    void clearSocketCallbacks();
};

void WebManager::Impl::init() {
    new EventListener([this]() {
        m_socketConnected = true;
		for (auto& callback : m_onSocketConnected) {
            callback();
        }
        m_onSocketConnected.clear();
		return ListenerResult::Propagate;
    }, DispatchFilter<>("alk.editorcollab/socket-connected"));

    new EventListener([this]() {
        m_socketConnected = false;
        for (auto& callback : m_onSocketDisconnected) {
            callback();
        }
        m_onSocketDisconnected.clear();
        return ListenerResult::Propagate;
    }, DispatchFilter<>("alk.editorcollab/socket-disconnected"));
}

web::WebRequest WebManager::Impl::createRequest() const {
    auto req = web::WebRequest();
    req.userAgent(fmt::format("Editor Collab/{} (Geode/{}, Collab API/{})",
        Mod::get()->getVersion().toVString(), Loader::get()->getVersion().toVString(), "NaN"
    ));
    req.timeout(std::chrono::seconds(10));
    return req;
}

web::WebRequest WebManager::Impl::createAuthenticatedRequest() const {
    auto token = AccountManager::get()->getLoginToken();
    auto req = this->createRequest();
    req.header("Authorization", fmt::format("Bearer {}", token));
    return req;
}

std::string WebManager::Impl::getServerURL() const {
    return "localhost:9100";
    // return "https://tulipalk.me/editor-collab/testing";
}

bool WebManager::Impl::isSocketConnected() const {
    return m_socketConnected;
}

void WebManager::Impl::runOnSocketConnected(std::function<void()>&& callback) {
    m_onSocketConnected.push_back(std::move(callback));
}

void WebManager::Impl::runOnSocketDisconnected(std::function<void()>&& callback) {
    m_onSocketDisconnected.push_back(std::move(callback));
}

void WebManager::Impl::clearSocketCallbacks() {
    m_onSocketConnected.clear();
    m_onSocketDisconnected.clear();
}

WebManager* WebManager::get() {
    static WebManager instance;
    static bool initialized = false;
    if (!initialized) {
        instance.impl->init();
        initialized = true;
    }
    return &instance;
}

WebManager::WebManager() : impl(std::make_unique<Impl>()) {}
WebManager::~WebManager() = default;

web::WebRequest WebManager::createRequest() const {
    return impl->createRequest();
}

std::string WebManager::getServerURL() const {
    return impl->getServerURL();
}

std::string WebManager::getServerURL(std::string_view path) const {
    return fmt::format("{}/{}", this->getServerURL(), path);
}

web::WebRequest WebManager::createAuthenticatedRequest() const {
    return impl->createAuthenticatedRequest();
}

bool WebManager::isSocketConnected() const {
    return impl->isSocketConnected();
}

void WebManager::runOnSocketConnected(std::function<void()>&& callback) {
    impl->runOnSocketConnected(std::move(callback));
}

void WebManager::runOnSocketDisconnected(std::function<void()>&& callback) {
    impl->runOnSocketDisconnected(std::move(callback));
}

void WebManager::clearSocketCallbacks() {
    impl->clearSocketCallbacks();
}