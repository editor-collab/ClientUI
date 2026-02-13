#include <managers/LevelManager.hpp>
#include <managers/WebManager.hpp>
#include <Geode/loader/Dispatch.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class LevelManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<std::string> m_hostedLevels;
    std::optional<std::string> m_joinedLevel;
    uint32_t m_clientId = 0;

    ListenerHandle m_levelKickedHandle;
    ListenerHandle m_updateSnapshotHandle;
    async::TaskHolder<Result<>> m_updateSnapshotTask;
    
    bool errorCallback(web::WebResponse* response);

    arc::Future<Result<LevelManager::CreateLevelResult>> createLevel(LevelSetting const& settings);
    arc::Future<Result<LevelManager::JoinLevelResult>> joinLevel(std::string_view levelKey);
    arc::Future<Result<>> leaveLevel(CameraValue const& camera);
    arc::Future<Result<>> deleteLevel(std::string_view levelKey);
    arc::Future<Result<std::vector<uint8_t>>> getSnapshot(std::string_view levelKey, std::string_view hash);
    arc::Future<Result<LevelEntry>> updateLevelSettings(std::string_view levelKey, LevelSetting const& settings);
    arc::Future<Result<>> updateLevelSnapshot(std::string_view levelKey, std::string_view token, std::span<uint8_t> snapshot);
    arc::Future<Result<>> kickUser(std::string_view levelKey, uint32_t accountId, std::string_view reason);
    void leaveLevelAbnormal();

    std::vector<std::string> getHostedLevels() const;
    std::optional<std::string> getJoinedLevelKey() const;
    uint32_t getClientId() const;
    bool isInLevel() const;

    arc::Future<Result<>> cancelReconnect();
};

std::vector<std::string> LevelManager::Impl::getHostedLevels() const {
    return m_hostedLevels;
}

uint32_t LevelManager::Impl::getClientId() const {
    return m_clientId;
}

bool LevelManager::Impl::isInLevel() const {
    return m_joinedLevel.has_value() && GJBaseGameLayer::get() != nullptr;
}

std::optional<std::string> LevelManager::Impl::getJoinedLevelKey() const {
    return m_joinedLevel;
}

arc::Future<Result<>> LevelManager::Impl::cancelReconnect() {
    Dispatch<std::string_view>("cancel-reconnect"_spr).send("Cancelled reconnect");

    co_return co_await this->leaveLevel(CameraValue{});
}

void LevelManager::Impl::leaveLevelAbnormal() {
    m_joinedLevel = std::nullopt;
}

// bool LevelManager::Impl::errorCallback(web::WebResponse* response) {
//     auto const res = response->string();
//     if (res.isErr()) {
//         m_joinedLevel.clear();
//         m_requestCallback(Err("Invalid string response"));
//         return true;
//     }
//     if (!response->ok()) {
//         m_joinedLevel.clear();
//         auto const code = response->code();
//         if (code == 401) {
//             m_requestCallback(Err("Invalid credentials"));
//         }
//         else {
//             m_requestCallback(Err(fmt::format("HTTP error: {}", code)));
//         }
//         return true;
//     }
//     return false;
// }

arc::Future<Result<LevelManager::CreateLevelResult>> LevelManager::Impl::createLevel(LevelSetting const& settings) {
    //////// log::debug("Creating level");

    auto const settingString = matjson::Value(settings).dump(matjson::NO_INDENTATION);

    std::vector<uint8_t> snapshot;
    Dispatch<std::vector<uint8_t>*>("get-level-snapshot"_spr).send(&snapshot);
    if (snapshot.empty()) {
        log::warn("No snapshot data available for new level");
    }

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("settings", settingString);
    req.body(ByteVector(snapshot.begin(), snapshot.end()));
    req.header("Content-Type", "application/octet-stream");
    auto response = co_await req.post(WebManager::get()->getServerURL("level/create"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    matjson::Value const values = GEODE_CO_UNWRAP(response.json());

    auto levelKey = GEODE_CO_UNWRAP(values["level-key"].asString());
    uint32_t const clientId = GEODE_CO_UNWRAP(values["client-id"].asInt());

    m_hostedLevels.push_back(levelKey);
    m_joinedLevel = levelKey;

    co_return Ok(LevelManager::CreateLevelResult{clientId, levelKey});
}
arc::Future<Result<LevelManager::JoinLevelResult>> LevelManager::Impl::joinLevel(std::string_view levelKey) {
    //////// log::debug("Joining level {}", levelKey);
    std::string levelKeyStr(levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKeyStr);
    auto response = co_await req.post(WebManager::get()->getServerURL("level/join"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    matjson::Value const values = GEODE_CO_UNWRAP(response.json());

    auto const hash = GEODE_CO_UNWRAP(values["snapshot-hash"].asString());
    uint32_t const clientId = GEODE_CO_UNWRAP(values["client-id"].asInt());
    auto camera = values["camera-value"].as<CameraValue>().unwrapOrDefault();

    auto snapshot = GEODE_CO_UNWRAP(co_await this->getSnapshot(levelKeyStr, hash));

    co_return Ok(LevelManager::JoinLevelResult{clientId, hash, std::move(snapshot), camera});
}
arc::Future<Result<>> LevelManager::Impl::leaveLevel(CameraValue const& camera) {
    //////// log::debug("Leaving level");
    m_joinedLevel = std::nullopt;

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("camera_x", numToString(camera.x));
    req.param("camera_y", numToString(camera.y));
    req.param("camera_zoom", numToString(camera.zoom));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/leave"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    
    co_return Ok();
}
arc::Future<Result<>> LevelManager::Impl::deleteLevel(std::string_view levelKey) {
    //////// log::debug("Deleting level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/delete"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    m_joinedLevel = std::nullopt;
    m_hostedLevels.erase(std::remove(m_hostedLevels.begin(), m_hostedLevels.end(), levelKey), m_hostedLevels.end());

    co_return Ok();
}
arc::Future<Result<std::vector<uint8_t>>> LevelManager::Impl::getSnapshot(std::string_view levelKey, std::string_view snapshotHash) {
    //////// log::debug("Getting snapshot for level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.param("snapshot_hash", std::string(snapshotHash));
    auto response = co_await req.get(WebManager::get()->getServerURL("level/get_snapshot"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return Ok(response.data());
}

arc::Future<Result<LevelEntry>> LevelManager::Impl::updateLevelSettings(std::string_view levelKey, LevelSetting const& settings) {
    //////// log::debug("Updating level settings for level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.bodyJSON(matjson::Value(settings));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/edit_settings"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    matjson::Value const values = GEODE_CO_UNWRAP(response.json());
    co_return Ok(GEODE_CO_UNWRAP(values["entry"].as<LevelEntry>()));
}

arc::Future<Result<>> LevelManager::Impl::updateLevelSnapshot(std::string_view levelKey, std::string_view token, std::span<uint8_t> snapshot) {
    //////// log::debug("Updating level snapshot for level {}", levelKey);

    auto req =  WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.param("request_token", std::string(token));
    req.body(ByteVector(snapshot.begin(), snapshot.end()));
    req.header("Content-Type", "application/octet-stream");
    auto response = co_await req.post(WebManager::get()->getServerURL("level/update_snapshot"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return Ok();
}

arc::Future<Result<>> LevelManager::Impl::kickUser(std::string_view levelKey, uint32_t accountId, std::string_view reason) {
    //////// log::debug("Kicking user {} from level {}", accountId, levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.param("account_id", accountId);
    req.param("reason", std::string(reason));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/kick_user"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return Ok();
}

LevelManager* LevelManager::get() {
    static LevelManager instance;
    return &instance;
}

LevelManager::LevelManager() : impl(std::make_unique<Impl>()) {}
LevelManager::~LevelManager() = default;

arc::Future<Result<LevelManager::CreateLevelResult>> LevelManager::createLevel(LevelSetting const& settings) {
    return impl->createLevel(settings);
}
arc::Future<Result<LevelManager::JoinLevelResult>> LevelManager::joinLevel(std::string_view levelKey) {
    return impl->joinLevel(levelKey);
}
arc::Future<Result<>> LevelManager::leaveLevel(CameraValue const& camera) {
    return impl->leaveLevel(camera);
}
arc::Future<Result<>> LevelManager::deleteLevel(std::string_view levelKey) {
    return impl->deleteLevel(levelKey);
}
arc::Future<Result<std::vector<uint8_t>>> LevelManager::getSnapshot(std::string_view levelKey, std::string_view hash) {
    return impl->getSnapshot(levelKey, hash);
}
arc::Future<Result<LevelEntry>> LevelManager::updateLevelSettings(std::string_view levelKey, LevelSetting const& settings) {
    return impl->updateLevelSettings(levelKey, settings);
}

arc::Future<Result<>> LevelManager::updateLevelSnapshot(std::string_view levelKey, std::string_view token, std::span<uint8_t> snapshot) {
    return impl->updateLevelSnapshot(levelKey, token, snapshot);
}

arc::Future<Result<>> LevelManager::kickUser(std::string_view levelKey, uint32_t accountId, std::string_view reason) {
    return impl->kickUser(levelKey, accountId, reason);
}

void LevelManager::leaveLevelAbnormal() {
    impl->leaveLevelAbnormal();
}

std::vector<std::string> LevelManager::getHostedLevels() const {
    return impl->getHostedLevels();
}

uint32_t LevelManager::getClientId() const {
    return impl->getClientId();
}

bool LevelManager::isInLevel() const {
    return impl->isInLevel();
}

std::optional<std::string> LevelManager::getJoinedLevelKey() const {
    return impl->getJoinedLevelKey();
}

void LevelManager::cancelReconnect() {
    impl->cancelReconnect();
}