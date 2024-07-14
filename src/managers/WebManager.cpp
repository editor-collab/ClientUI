#include <managers/WebManager.hpp>
#include <chrono>

using namespace tulip::editor;
using namespace geode::prelude;

class WebManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    web::WebRequest createRequest() const;
    std::string getServerURL() const;
};

web::WebRequest WebManager::Impl::createRequest() const {
    auto req = web::WebRequest();
    req.userAgent(fmt::format("Editor Collab/{} (Geode/{}, Collab API/{})",
        Mod::get()->getVersion().toString(), Loader::get()->getVersion().toString(), "NaN"
    ));
    req.timeout(std::chrono::seconds(15));
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