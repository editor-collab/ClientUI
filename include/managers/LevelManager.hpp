#pragma once
#include <Geode/Geode.hpp>
#include <memory>
#include "../data/LevelEntry.hpp"
#include "../data/CameraValue.hpp"
#include <Geode/utils/web.hpp>

namespace tulip::editor {
    using geode::Result;
    using geode::Task;
    using geode::utils::web::WebProgress;
    enum class SocketStatus {
        Connected,
        Disconnected,
        Reconnecting,
    };

    class LevelManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        LevelManager();
        ~LevelManager();

    public:
        static LevelManager* get();

        struct CreateLevelResult {
            uint32_t clientId;
            std::string levelKey;
        };

        struct JoinLevelResult {
            uint32_t clientId;
            std::string snapshotHash;
            std::vector<uint8_t> snapshot;
            std::optional<CameraValue> camera;
        };

        arc::Future<Result<CreateLevelResult>> createLevel(LevelSetting settings);
        arc::Future<Result<JoinLevelResult>> joinLevel(std::string levelKey, arc::CancellationToken& cancelToken);
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
}