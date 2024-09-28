#include <managers/FetchManager.hpp>
#include <managers/WebManager.hpp>
#include <data/LevelEntry.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class FetchManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;
    
    bool errorCallback(web::WebResponse* response);

    Result<std::vector<LevelEntry>> parseLevels(web::WebResponse* response);

    Task<Result<std::vector<LevelEntry>>, WebProgress> getMyLevels();
    Task<Result<std::vector<LevelEntry>>, WebProgress> getSharedWithMe();
    Task<Result<std::vector<LevelEntry>>, WebProgress> getDiscover();
};

Result<std::vector<LevelEntry>> FetchManager::Impl::parseLevels(web::WebResponse* response) {
    if (!response->ok()) return Err(fmt::format("HTTP error: {}", response->code()));

    auto res = response->json();
    if (res.isErr()) return Err("Invalid JSON response");

    log::debug("Response: {}", res.unwrap().dump());

    auto json = res.unwrap();
    if (json.is<std::vector<LevelEntry>>()) return Ok(json.as<std::vector<LevelEntry>>());
    return Err("Invalid JSON response");
}

Task<Result<std::vector<LevelEntry>>, WebProgress> FetchManager::Impl::getMyLevels() {
    log::debug("Fetching my levels");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto task = req.get(WebManager::get()->getServerURL("fetch/my_levels"));
    auto ret = task.map([=, this](auto response) -> Result<std::vector<LevelEntry>> {
        return this->parseLevels(response);
    });
    return ret;
}

Task<Result<std::vector<LevelEntry>>, WebProgress> FetchManager::Impl::getSharedWithMe() {
    log::debug("Fetching shared levels");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto task = req.get(WebManager::get()->getServerURL("fetch/shared_with_me"));
    auto ret = task.map([=, this](auto response) -> Result<std::vector<LevelEntry>> {
        return this->parseLevels(response);
    });
    return ret;
}

Task<Result<std::vector<LevelEntry>>, WebProgress> FetchManager::Impl::getDiscover() {
    log::debug("Fetching discover levels");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto task = req.get(WebManager::get()->getServerURL("fetch/discover"));
    auto ret = task.map([=, this](auto response) -> Result<std::vector<LevelEntry>> {
        return this->parseLevels(response);
    });
    return ret;
}

FetchManager* FetchManager::get() {
    static FetchManager instance;
    return &instance;
}

FetchManager::FetchManager() : impl(std::make_unique<Impl>()) {}

FetchManager::~FetchManager() = default;

Task<Result<std::vector<LevelEntry>>, WebProgress> FetchManager::getMyLevels() {
    return impl->getMyLevels();
}

Task<Result<std::vector<LevelEntry>>, WebProgress> FetchManager::getSharedWithMe() {
    return impl->getSharedWithMe();
}

Task<Result<std::vector<LevelEntry>>, WebProgress> FetchManager::getDiscover() {
    return impl->getDiscover();
}