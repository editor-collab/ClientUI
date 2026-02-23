#include <managers/LevelManager.hpp>
#include <managers/WebManager.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <arc/future/Select.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class LevelManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    asp::Mutex<std::string> m_joinedLevel;
    uint32_t m_clientId = 0;

    ListenerHandle m_levelKickedHandle;
    ListenerHandle m_updateSnapshotHandle;
    async::TaskHolder<Result<>> m_updateSnapshotTask;

    asp::Mutex<SocketStatus> m_socketStatus = SocketStatus::Disconnected;
    
    bool errorCallback(web::WebResponse* response);

    arc::Future<Result<LevelManager::CreateLevelResult>> createLevel(LevelSetting settings);
    arc::Future<Result<LevelManager::JoinLevelResult>> joinLevel(std::string levelKey, arc::CancellationToken& cancelToken);
    arc::Future<Result<>> leaveLevel(CameraValue camera);
    arc::Future<Result<>> deleteLevel(std::string levelKey);
    arc::Future<Result<std::vector<uint8_t>>> getSnapshot(std::string levelKey, std::string hash);
    arc::Future<Result<LevelEntry>> updateLevelSettings(std::string levelKey, LevelSetting settings);
    arc::Future<Result<>> updateLevelSnapshot(std::string levelKey, std::string token, std::vector<uint8_t> snapshot);
    arc::Future<Result<>> kickUser(std::string levelKey, uint32_t accountId, std::string reason);
    void leaveLevelAbnormal();

    std::string getJoinedLevelKey() const;
    uint32_t getClientId() const;
    bool hasJoinedLevelKey() const;

    arc::Future<Result<>> cancelReconnect();
    SocketStatus getSocketStatus() const;
    void setSocketStatus(SocketStatus status);
};

SocketStatus LevelManager::Impl::getSocketStatus() const {
    return *m_socketStatus.lock();
}

void LevelManager::Impl::setSocketStatus(SocketStatus status) {
    m_socketStatus.lock() = status;
}

uint32_t LevelManager::Impl::getClientId() const {
    return m_clientId;
}

bool LevelManager::Impl::hasJoinedLevelKey() const {
    return !m_joinedLevel.lock()->empty();
}

std::string LevelManager::Impl::getJoinedLevelKey() const {
    return *m_joinedLevel.lock();
}

arc::Future<Result<>> LevelManager::Impl::cancelReconnect() {
    Dispatch<std::string>("alk.editor-collab/cancel-reconnect").send("Cancelled reconnect");

    co_return co_await this->leaveLevel(CameraValue{});
}

void LevelManager::Impl::leaveLevelAbnormal() {
    m_joinedLevel.lock() = "";
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

arc::Future<Result<LevelManager::CreateLevelResult>> LevelManager::Impl::createLevel(LevelSetting settings) {
    //////// log::debug("Creating level");

    auto const settingString = matjson::Value(settings).dump(matjson::NO_INDENTATION);

    std::vector<uint8_t> snapshot;
    Dispatch<std::vector<uint8_t>*>("alk.editor-collab/get-level-snapshot").send(&snapshot);
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

    m_joinedLevel.lock() = levelKey;

    co_return Ok(LevelManager::CreateLevelResult{clientId, levelKey});
}
arc::Future<Result<LevelManager::JoinLevelResult>> LevelManager::Impl::joinLevel(std::string levelKey, arc::CancellationToken& cancelToken) {
    //////// log::debug("Joining level {}", levelKey);
    bool cancelled = false;

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKey);

    web::WebResponse response;

    co_await arc::select(
        arc::selectee(cancelToken.waitCancelled(), [&](){ cancelled = true; }),
        arc::selectee(req.post(WebManager::get()->getServerURL("level/join")), [&](auto res) {
            response = std::move(res);
        })
    );
    if (cancelled) {
        co_return Err("Join level cancelled");
    }

    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    matjson::Value const values = GEODE_CO_UNWRAP(response.json());

    auto const hash = GEODE_CO_UNWRAP(values["snapshot-hash"].asString());
    uint32_t const clientId = GEODE_CO_UNWRAP(values["client-id"].asInt());
    auto camera = values["camera-value"].as<CameraValue>().unwrapOrDefault();

    std::vector<uint8_t> snapshot;
    std::string error;
    
    co_await arc::select(
        arc::selectee(cancelToken.waitCancelled(), [&](){ cancelled = true; }),
        arc::selectee(this->getSnapshot(levelKey, hash), [&](auto res) {
            if (GEODE_UNWRAP_EITHER(snap, err, res)) {
                snapshot = std::move(snap);
            }
            else {
                error = std::move(err);
            }
        })
    );
    if (!error.empty()) {
        co_return Err(std::move(error));
    }
    if (cancelled) {
        co_return Err("Join level cancelled");
    }

    m_joinedLevel.lock() = levelKey;
    this->setSocketStatus(SocketStatus::Reconnecting);

    co_return Ok(LevelManager::JoinLevelResult{clientId, hash, std::move(snapshot), camera});
}
arc::Future<Result<>> LevelManager::Impl::leaveLevel(CameraValue camera) {
    //////// log::debug("Leaving level");
    m_joinedLevel.lock() = "";

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("camera_x", numToString(camera.x));
    req.param("camera_y", numToString(camera.y));
    req.param("camera_zoom", numToString(camera.zoom));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/leave"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    
    co_return Ok();
}
arc::Future<Result<>> LevelManager::Impl::deleteLevel(std::string levelKey) {
    //////// log::debug("Deleting level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/delete"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    m_joinedLevel.lock() = "";

    co_return Ok();
}
arc::Future<Result<std::vector<uint8_t>>> LevelManager::Impl::getSnapshot(std::string levelKey, std::string snapshotHash) {
    //////// log::debug("Getting snapshot for level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.param("snapshot_hash", std::string(snapshotHash));
    auto response = co_await req.get(WebManager::get()->getServerURL("level/get_snapshot"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return Ok(response.data());
}

arc::Future<Result<LevelEntry>> LevelManager::Impl::updateLevelSettings(std::string levelKey, LevelSetting settings) {
    //////// log::debug("Updating level settings for level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.bodyJSON(matjson::Value(settings));
    auto response = co_await req.post(WebManager::get()->getServerURL("level/edit_settings"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    matjson::Value const values = GEODE_CO_UNWRAP(response.json());
    co_return Ok(GEODE_CO_UNWRAP(values["entry"].as<LevelEntry>()));
}

arc::Future<Result<>> LevelManager::Impl::updateLevelSnapshot(std::string levelKey, std::string token, std::vector<uint8_t> snapshot) {
    //////// log::debug("Updating level snapshot for level {}", levelKey);

    auto req =  WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", std::string(levelKey));
    req.param("request_token", std::string(token));
    req.body(std::move(snapshot));
    req.header("Content-Type", "application/octet-stream");
    auto response = co_await req.post(WebManager::get()->getServerURL("level/update_snapshot"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));
    co_return Ok();
}

arc::Future<Result<>> LevelManager::Impl::kickUser(std::string levelKey, uint32_t accountId, std::string reason) {
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

arc::Future<Result<LevelManager::CreateLevelResult>> LevelManager::createLevel(LevelSetting settings) {
    return impl->createLevel(std::move(settings));
}
arc::Future<Result<LevelManager::JoinLevelResult>> LevelManager::joinLevel(std::string levelKey, arc::CancellationToken& cancelToken) {
    return impl->joinLevel(std::move(levelKey), cancelToken);
}
arc::Future<Result<>> LevelManager::leaveLevel(CameraValue camera) {
    return impl->leaveLevel(std::move(camera));
}
arc::Future<Result<>> LevelManager::deleteLevel(std::string levelKey) {
    return impl->deleteLevel(std::move(levelKey));
}
arc::Future<Result<std::vector<uint8_t>>> LevelManager::getSnapshot(std::string levelKey, std::string hash) {
    return impl->getSnapshot(std::move(levelKey), std::move(hash));
}
arc::Future<Result<LevelEntry>> LevelManager::updateLevelSettings(std::string levelKey, LevelSetting settings) {
    return impl->updateLevelSettings(std::move(levelKey), std::move(settings));
}

arc::Future<Result<>> LevelManager::updateLevelSnapshot(std::string levelKey, std::string token, std::vector<uint8_t> snapshot) {
    return impl->updateLevelSnapshot(std::move(levelKey), std::move( token), std::move(snapshot));
}

arc::Future<Result<>> LevelManager::kickUser(std::string levelKey, uint32_t accountId, std::string reason) {
    return impl->kickUser(std::move(levelKey), accountId, std::move(reason));
}

void LevelManager::leaveLevelAbnormal() {
    impl->leaveLevelAbnormal();
}

uint32_t LevelManager::getClientId() const {
    return impl->getClientId();
}

bool LevelManager::hasJoinedLevelKey() const {
    return impl->hasJoinedLevelKey();
}

std::string LevelManager::getJoinedLevelKey() const {
    return impl->getJoinedLevelKey();
}

arc::Future<Result<>> LevelManager::cancelReconnect() {
    return impl->cancelReconnect();
}

SocketStatus LevelManager::getSocketStatus() const {
    return impl->getSocketStatus();
}

void LevelManager::setSocketStatus(SocketStatus status) {
    impl->setSocketStatus(status);
}