#include <managers/FetchManager.hpp>
#include <managers/LocalManager.hpp>
#include <managers/WebManager.hpp>
#include <data/LevelEntry.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class FetchManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;
    
    bool errorCallback(web::WebResponse* response);

    size_t m_hostableCount = 0;

    Result<std::vector<LevelEntry>> parseLevels(web::WebResponse* response);

    arc::Future<Result<std::vector<LevelEntry>>> getMyLevels();
    std::vector<LevelEntry> const& getLastMyLevels();
    arc::Future<Result<std::vector<LevelEntry>>> getSharedWithMe();
    arc::Future<Result<std::vector<LevelEntry>>> getDiscover();

    size_t getHostableCount() const {
        return m_hostableCount;
    }
    void addHostableCount(size_t count) {
        m_hostableCount += count;
    }
};

Result<std::vector<LevelEntry>> FetchManager::Impl::parseLevels(web::WebResponse* response) {
    if (!response->ok()) return Err(fmt::format("HTTP error: {}", response->code()));

    auto json = GEODE_UNWRAP(response->json());

    if (!json.contains("levels")) return Err("Invalid response");

    return json["levels"].as<std::vector<LevelEntry>>();
}

arc::Future<Result<std::vector<LevelEntry>>> FetchManager::Impl::getMyLevels() {
    //////// log::debug("Fetching my levels");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto response = co_await req.get(WebManager::get()->getServerURL("fetch/my_levels"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    auto levels = this->parseLevels(&response);
    if (levels.isOk()) {
        matjson::Value json = GEODE_CO_UNWRAP(response.json());
        m_hostableCount = json["hostable-count"].as<size_t>().unwrapOr(0);
    }
    co_return levels;
}

arc::Future<Result<std::vector<LevelEntry>>> FetchManager::Impl::getSharedWithMe() {
    //////// log::debug("Fetching shared levels");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto response = co_await req.get(WebManager::get()->getServerURL("fetch/shared_with_me"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return this->parseLevels(&response);
}

arc::Future<Result<std::vector<LevelEntry>>> FetchManager::Impl::getDiscover() {
    //////// log::debug("Fetching discover levels");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto response = co_await req.get(WebManager::get()->getServerURL("fetch/discover"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return this->parseLevels(&response);
}

FetchManager* FetchManager::get() {
    static FetchManager instance;
    return &instance;
}

FetchManager::FetchManager() : impl(std::make_unique<Impl>()) {}

FetchManager::~FetchManager() = default;

arc::Future<Result<std::vector<LevelEntry>>> FetchManager::getMyLevels() {
    return impl->getMyLevels();
}

arc::Future<Result<std::vector<LevelEntry>>> FetchManager::getSharedWithMe() {
    return impl->getSharedWithMe();
}

arc::Future<Result<std::vector<LevelEntry>>> FetchManager::getDiscover() {
    return impl->getDiscover();
}

size_t FetchManager::getHostableCount() const {
    return impl->getHostableCount();
}

void FetchManager::addHostableCount(size_t count) {
    impl->addHostableCount(count);
}