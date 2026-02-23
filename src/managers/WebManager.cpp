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
    std::string m_loginToken;

    geode::Result<> errorCallback(geode::utils::web::WebResponse* response);
    web::WebRequest createRequest() const;
    web::WebRequest createAuthenticatedRequest() const;
    std::string getServerURL() const;

    void setLoginToken(std::string token) {
        m_loginToken = std::move(token);
    }

    std::string getLoginToken() const {
        return m_loginToken;
    }

    bool isSocketConnected() const;
    void connectSocket();
    void disconnectSocket();
};

void WebManager::Impl::connectSocket() {
    m_socketConnected = true;
}

void WebManager::Impl::disconnectSocket() {
    m_socketConnected = false;
}

geode::Result<> WebManager::Impl::errorCallback(web::WebResponse* response) {
    auto const res = response->string();
    if (res.isErr()) {
        return Err("Invalid string response");
    }
    if (!response->ok()) {
        auto const code = response->code();
        switch (code) {
            case 400:
                return Err("Bad request: {}", res.unwrapOrDefault());
            case 401:
                return Err("Unauthorized: {}", res.unwrapOrDefault());
            case 404:
                return Err("Not found: {}", res.unwrapOrDefault());
            case 403:
                return Err("Forbidden: {}", res.unwrapOrDefault());
            case 500:
                return Err("Internal server error: {}", res.unwrapOrDefault());
            case 502:
                return Err("Bad gateway: {}", res.unwrapOrDefault());
            default:
                return Err(fmt::format("Error {}: {}", code, res.unwrapOrDefault()));
        }
    }
    return Ok();
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
    auto token = this->getLoginToken();
    auto req = this->createRequest();
    req.header("Authorization", fmt::format("Bearer {}", token));
    return req;
}

std::string WebManager::Impl::getServerURL() const {
    auto local = Mod::get()->getSettingValue<bool>("use-local-server");
    auto testing = Mod::get()->getSettingValue<bool>("use-testing-server");
    if (local) {
        return "http://localhost:9101/v1";
    }
    else if (testing) {
        return "https://tulipalk.me/editor-collab/testing/v1";
    }
    else {
        return "https://tulipalk.me/editor-collab/release/v1";
    }
}

bool WebManager::Impl::isSocketConnected() const {
    return m_socketConnected;
}

WebManager* WebManager::get() {
    static WebManager instance;
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

geode::Result<> WebManager::errorCallback(web::WebResponse* response) {
    return impl->errorCallback(response);
}

void WebManager::setLoginToken(std::string token) {
    impl->setLoginToken(std::move(token));
}

std::string WebManager::getLoginToken() const {
    return impl->getLoginToken();
}