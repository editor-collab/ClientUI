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

    bool m_socketConnected = false;

    void init();

    EventListener<DispatchFilter<>> m_socketConnectedListener = DispatchFilter<>("alk.editorcollab/socket-connected");
    EventListener<DispatchFilter<>> m_socketDisconnectedListener = DispatchFilter<>("alk.editorcollab/socket-disconnected");
    EventListener<DispatchFilter<>> m_socketAbnormallyDisconnectedListener = DispatchFilter<>("alk.editorcollab/socket-abnormally-disconnected");

    web::WebRequest createRequest() const;
    web::WebRequest createAuthenticatedRequest() const;
    std::string getServerURL() const;

    bool isSocketConnected() const;
};

void WebManager::Impl::init() {
    m_socketConnectedListener.bind([this]() {
        m_socketConnected = true;

        return ListenerResult::Propagate;
    });

    m_socketDisconnectedListener.bind([this]() {
        m_socketConnected = false;

        return ListenerResult::Propagate;
    });

    m_socketAbnormallyDisconnectedListener.bind([this]() {
        m_socketConnected = false;

        return ListenerResult::Propagate;
    });
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
