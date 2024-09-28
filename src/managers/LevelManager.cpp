#include <managers/LevelManager.hpp>
#include <managers/WebManager.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class LevelManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<std::string> m_hostedLevels;
    std::string m_joinedLevel;
    uint32_t m_clientId = 0;
    
    bool errorCallback(web::WebResponse* response);

    Task<Result<std::pair<uint32_t, std::string>>, WebProgress> createLevel(int slotId, int uniqueId);
    Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, WebProgress> joinLevel(std::string_view levelKey);
    Task<Result<>, WebProgress> leaveLevel();
    Task<Result<>, WebProgress> deleteLevel(std::string_view levelKey);
    Task<Result<std::vector<uint8_t>>, WebProgress> getSnapshot(std::string_view levelKey, std::string_view hash);

    std::vector<std::string> getHostedLevels() const;
    uint32_t getClientId() const;
    bool isInLevel() const;
};

std::vector<std::string> LevelManager::Impl::getHostedLevels() const {
    return m_hostedLevels;
}

uint32_t LevelManager::Impl::getClientId() const {
    return m_clientId;
}

bool LevelManager::Impl::isInLevel() const {
    return !m_joinedLevel.empty();
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

Task<Result<std::pair<uint32_t, std::string>>, WebProgress> LevelManager::Impl::createLevel(int slotId, int uniqueId) {
    log::debug("Creating level");

    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("slot_id", slotId);
    req.param("unique_id", uniqueId);
    auto task = req.post(WebManager::get()->getServerURL("level/create"));
    auto ret = task.map([=, this](auto response) -> Result<std::pair<uint32_t, std::string>> {
        if (response->ok()) {
            auto const res = response->string().unwrap();

            auto const values = string::split(res, ":");
            if (values.size() != 2) {
                return Err("Invalid response");
            }
            auto const levelKey = values[0];
            if (auto const clientId = numFromString<uint32_t>(values[1]); clientId.isOk()) {
                m_hostedLevels.push_back(levelKey);
                m_joinedLevel = levelKey;
                return Ok(std::make_pair(clientId.unwrap(), levelKey));
            }
            return Err("Invalid client ID");
        }
        return Err(fmt::format("HTTP error: {}", response->code()));
    });
    return ret;
}
Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, WebProgress> LevelManager::Impl::joinLevel(std::string_view levelKey) {
    log::debug("Joining level {}", levelKey);

    auto ret = Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, WebProgress>::runWithCallback([=, this](auto finish, auto progressC, auto hasBeenCancelled) {
        auto req = WebManager::get()->createAuthenticatedRequest();
        req.param("level_key", levelKey);
        auto task = req.post(WebManager::get()->getServerURL("level/join"));
        auto task2 = task.map([=, this](auto response) -> Result<std::pair<std::string, uint32_t>> {
            if (response->ok()) {
                auto const res = response->string().unwrap();

                auto const values = string::split(res, ":");
                if (values.size() != 2) {
                    return Err("Invalid response");
                }
                auto const hash = values[0];
                if (auto const clientId = numFromString<uint32_t>(values[1]); clientId.isOk()) {
                    log::debug("Joined level {} with client ID {}", levelKey, clientId.unwrap());
                    log::debug("Snapshot hash: {}", hash);
                    return Ok(std::make_pair(hash, clientId.unwrap()));
                }
                return Err("Invalid client ID");
            }
            return Err(fmt::format("HTTP error: {}", response->code()));
        });
        task2.listen([=, this](auto* result) {
            if (result->isOk()) {
                auto const clientId = result->unwrap().second;
                auto task = this->getSnapshot(levelKey, result->unwrap().first);
                task.listen([=, this](auto* result) {
                    if (result->isOk()) {
                        m_joinedLevel = levelKey;
                        finish(Ok(std::make_pair(clientId, std::move(result->unwrap()))));
                    }
                    else {
                        finish(Err(result->unwrapErr()));
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
Task<Result<>, WebProgress> LevelManager::Impl::leaveLevel() {
    log::debug("Leaving level");

    auto req = WebManager::get()->createAuthenticatedRequest();
    auto task = req.post(WebManager::get()->getServerURL("level/leave"));
    auto ret = task.map([=, this](auto response) -> Result<> {
        if (response->ok()) {
            m_joinedLevel.clear();
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

LevelManager* LevelManager::get() {
    static LevelManager instance;
    return &instance;
}

LevelManager::LevelManager() : impl(std::make_unique<Impl>()) {}
LevelManager::~LevelManager() = default;

Task<Result<std::pair<uint32_t, std::string>>, WebProgress> LevelManager::createLevel(int slotId, int uniqueId) {
    return impl->createLevel(slotId, uniqueId);
}
Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, WebProgress> LevelManager::joinLevel(std::string_view levelKey) {
    return impl->joinLevel(levelKey);
}
Task<Result<>, WebProgress> LevelManager::leaveLevel() {
    return impl->leaveLevel();
}
Task<Result<>, WebProgress> LevelManager::deleteLevel(std::string_view levelKey) {
    return impl->deleteLevel(levelKey);
}
Task<Result<std::vector<uint8_t>>, WebProgress> LevelManager::getSnapshot(std::string_view levelKey, std::string_view hash) {
    return impl->getSnapshot(levelKey, hash);
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