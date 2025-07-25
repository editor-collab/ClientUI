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

    EventListener<DispatchFilter<std::string_view>> m_levelKickedListener = DispatchFilter<std::string_view>("alk.editor-collab/level-kicked");
    EventListener<DispatchFilter<std::string_view>> m_updateSnapshotListener = DispatchFilter<std::string_view>("alk.editor-collab-ui/update-level-snapshot");
    EventListener<Task<Result<>, WebProgress>> m_updateSnapshotListenerTask;

    void init();
    
    bool errorCallback(web::WebResponse* response);

    Task<Result<LevelManager::CreateLevelResult>, WebProgress> createLevel(int slotId, LevelSetting const& settings);
    Task<Result<LevelManager::JoinLevelResult>, WebProgress> joinLevel(std::string_view levelKey);
    Task<Result<>, WebProgress> leaveLevel(CameraValue const& camera);
    Task<Result<>, WebProgress> deleteLevel(std::string_view levelKey);
    Task<Result<std::vector<uint8_t>>, WebProgress> getSnapshot(std::string_view levelKey, std::string_view hash);
    Task<Result<LevelEntry>, WebProgress> updateLevelSettings(std::string_view levelKey, LevelSetting const& settings);
    Task<Result<>, WebProgress> updateLevelSnapshot(std::string_view levelKey, std::string_view token, std::span<uint8_t> snapshot);
    Task<Result<>, WebProgress> kickUser(std::string_view levelKey, uint32_t accountId, std::string_view reason);

    std::vector<std::string> getHostedLevels() const;
    std::optional<std::string> getJoinedLevelKey() const;
    uint32_t getClientId() const;
    bool isInLevel() const;
};

void LevelManager::Impl::init() {
    m_levelKickedListener.bind([this](std::string_view reason) {
        m_joinedLevel = std::nullopt;
        return ListenerResult::Propagate;
    });

    m_updateSnapshotListener.bind([this](std::string_view token) {
        std::vector<uint8_t> snapshot;
        DispatchEvent<std::vector<uint8_t>*>("alk.editor-collab-ui/get-level-snapshot", &snapshot).post();
        if (snapshot.empty()) {
            log::warn("No snapshot data available for update");
            return ListenerResult::Propagate;
        }
        m_updateSnapshotListenerTask.setFilter(LevelManager::get()->updateLevelSnapshot(m_joinedLevel.value(), token, snapshot));
        return ListenerResult::Propagate;
    });
}

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

Task<Result<LevelManager::CreateLevelResult>, WebProgress> LevelManager::Impl::createLevel(int slotId, LevelSetting const& settings) {
    log::debug("Creating level");

    auto const settingString = matjson::Value(settings).dump(matjson::NO_INDENTATION);

    std::vector<uint8_t> snapshot;
    DispatchEvent<std::vector<uint8_t>*>("alk.editor-collab-ui/get-level-snapshot", &snapshot).post();
    if (snapshot.empty()) {
        log::warn("No snapshot data available for new level");
    }

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("slot_id", slotId);
    req.param("settings", settingString);
    req.body(ByteVector(snapshot.begin(), snapshot.end()));
    req.header("Content-Type", "application/octet-stream");
    auto task = req.post(WebManager::get()->getServerURL("level/create"));
    auto ret = task.map([=, this](auto response) -> Result<LevelManager::CreateLevelResult> {
        if (response->ok()) {
            matjson::Value const values = GEODE_UNWRAP(response->json());

            auto levelKey = GEODE_UNWRAP(values["level-key"].asString());
            uint32_t const clientId = GEODE_UNWRAP(values["client-id"].asInt());

            m_hostedLevels.push_back(levelKey);
            m_joinedLevel = levelKey;

            return Ok(LevelManager::CreateLevelResult{clientId, levelKey});
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}
Task<Result<LevelManager::JoinLevelResult>, WebProgress> LevelManager::Impl::joinLevel(std::string_view levelKey) {
    log::debug("Joining level {}", levelKey);
    std::string levelKeyStr(levelKey);

    auto ret = Task<Result<LevelManager::JoinLevelResult>, WebProgress>::runWithCallback([=, this](auto finish, auto progressC, auto hasBeenCancelled) {
        auto req = WebManager::get()->createAuthenticatedRequest();
        req.param("level_key", levelKeyStr);
        auto task = req.post(WebManager::get()->getServerURL("level/join"));
        auto task2 = task.map([=, this](auto response) -> Result<LevelManager::JoinLevelResult> {
            if (response->ok()) {
                matjson::Value const values = GEODE_UNWRAP(response->json());

                auto const hash = GEODE_UNWRAP(values["snapshot-hash"].asString());
                uint32_t const clientId = GEODE_UNWRAP(values["client-id"].asInt());
                auto camera = values["camera-value"].as<CameraValue>().unwrapOrDefault();

                log::debug("Joined level {} with client ID {}", levelKeyStr, clientId);
                log::debug("Snapshot hash: {}", hash);
                
                return Ok(LevelManager::JoinLevelResult{clientId, hash, {}, camera});
            }
            return Err(fmt::format("HTTP error: {}", response->code()));
        });
        task2.listen([=, this](auto* resultp) {
            if (GEODE_UNWRAP_IF_OK(values, *resultp)) {
                auto task = this->getSnapshot(levelKeyStr, values.snapshotHash);
                log::debug("task for snapshot created");
                task.listen([=, this](auto* result2p) {
                    log::debug("task for snapshot listened");
                    if (GEODE_UNWRAP_EITHER(snapshot, err, *result2p)) {
                        auto values2 = values;
                        log::debug("snapshot okay");
                        m_joinedLevel = levelKeyStr;
                        values2.snapshot = std::move(snapshot);
                        log::debug("snapshot set");
                        finish(Ok(std::move(values2)));
                    }
                    else {
                        finish(Err(err));
                    }
                }, [=](auto* progress) {
                    progressC(*progress);
                });
            }
        }, [=](auto* progress) {
            progressC(*progress);
        });
    });
    return ret;
}
Task<Result<>, WebProgress> LevelManager::Impl::leaveLevel(CameraValue const& camera) {
    log::debug("Leaving level");

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("camera_x", numToString(camera.x));
    req.param("camera_y", numToString(camera.y));
    req.param("camera_zoom", numToString(camera.zoom));
    auto task = req.post(WebManager::get()->getServerURL("level/leave"));
    auto ret = task.map([=, this](auto response) -> Result<> {
        if (response->ok()) {
            m_joinedLevel = std::nullopt;
            return Ok();
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}
Task<Result<>, WebProgress> LevelManager::Impl::deleteLevel(std::string_view levelKey) {
    log::debug("Deleting level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKey);
    auto task = req.post(WebManager::get()->getServerURL("level/delete"));
    auto ret = task.map([=, this](auto response) -> Result<> {
        if (response->ok()) {
            m_joinedLevel = std::nullopt;
            m_hostedLevels.erase(std::remove(m_hostedLevels.begin(), m_hostedLevels.end(), levelKey), m_hostedLevels.end());
            return Ok();
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}
Task<Result<std::vector<uint8_t>>, WebProgress> LevelManager::Impl::getSnapshot(std::string_view levelKey, std::string_view snapshotHash) {
    log::debug("Getting snapshot for level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKey);
    req.param("snapshot_hash", snapshotHash);
    auto task = req.get(WebManager::get()->getServerURL("level/get_snapshot"));
    auto ret = task.map([](web::WebResponse* response) -> Result<std::vector<uint8_t>> {
        if (response->ok()) {
            return Ok(response->data());
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}

Task<Result<LevelEntry>, WebProgress> LevelManager::Impl::updateLevelSettings(std::string_view levelKey, LevelSetting const& settings) {
    log::debug("Updating level settings for level {}", levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKey);
    req.bodyJSON(matjson::Value(settings));
    auto task = req.post(WebManager::get()->getServerURL("level/edit_settings"));
    auto ret = task.map([](web::WebResponse* response) -> Result<LevelEntry> {
        if (response->ok()) {
            matjson::Value const values = GEODE_UNWRAP(response->json());
            return Ok(GEODE_UNWRAP(values["entry"].as<LevelEntry>()));
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}

Task<Result<>, WebProgress> LevelManager::Impl::updateLevelSnapshot(std::string_view levelKey, std::string_view token, std::span<uint8_t> snapshot) {
    log::debug("Updating level snapshot for level {}", levelKey);

    auto req =  WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKey);
    req.param("request_token", token);
    req.body(ByteVector(snapshot.begin(), snapshot.end()));
    req.header("Content-Type", "application/octet-stream");
    auto task = req.post(WebManager::get()->getServerURL("level/update_snapshot"));
    auto ret = task.map([](web::WebResponse* response) -> Result<> {
        if (response->ok()) {
            return Ok();
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}

Task<Result<>, WebProgress> LevelManager::Impl::kickUser(std::string_view levelKey, uint32_t accountId, std::string_view reason) {
    log::debug("Kicking user {} from level {}", accountId, levelKey);

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("level_key", levelKey);
    req.param("account_id", accountId);
    req.param("reason", reason);
    auto task = req.post(WebManager::get()->getServerURL("level/kick_user"));
    auto ret = task.map([](web::WebResponse* response) -> Result<> {
        if (response->ok()) {
            return Ok();
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}

LevelManager* LevelManager::get() {
    static LevelManager instance;
    static bool initialized = false;
    if (!initialized) {
        instance.impl->init();
        initialized = true;
    }
    return &instance;
}

LevelManager::LevelManager() : impl(std::make_unique<Impl>()) {}
LevelManager::~LevelManager() = default;

Task<Result<LevelManager::CreateLevelResult>, WebProgress> LevelManager::createLevel(int slotId, LevelSetting const& settings) {
    return impl->createLevel(slotId, settings);
}
Task<Result<LevelManager::JoinLevelResult>, WebProgress> LevelManager::joinLevel(std::string_view levelKey) {
    return impl->joinLevel(levelKey);
}
Task<Result<>, WebProgress> LevelManager::leaveLevel(CameraValue const& camera) {
    return impl->leaveLevel(camera);
}
Task<Result<>, WebProgress> LevelManager::deleteLevel(std::string_view levelKey) {
    return impl->deleteLevel(levelKey);
}
Task<Result<std::vector<uint8_t>>, WebProgress> LevelManager::getSnapshot(std::string_view levelKey, std::string_view hash) {
    return impl->getSnapshot(levelKey, hash);
}
Task<Result<LevelEntry>, WebProgress> LevelManager::updateLevelSettings(std::string_view levelKey, LevelSetting const& settings) {
    return impl->updateLevelSettings(levelKey, settings);
}

Task<Result<>, WebProgress> LevelManager::updateLevelSnapshot(std::string_view levelKey, std::string_view token, std::span<uint8_t> snapshot) {
    return impl->updateLevelSnapshot(levelKey, token, snapshot);
}

Task<Result<>, WebProgress> LevelManager::kickUser(std::string_view levelKey, uint32_t accountId, std::string_view reason) {
    return impl->kickUser(levelKey, accountId, reason);
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