#include <managers/WebManager.hpp>
#include <managers/AccountManager.hpp>
#include <chrono>

using namespace tulip::editor;
using namespace geode::prelude;

class WebManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    web::WebRequest createRequest() const;
    web::WebRequest createAuthenticatedRequest() const;
    std::string getServerURL() const;
};

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