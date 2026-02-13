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

        arc::Future<Result<CreateLevelResult>> createLevel(LevelSetting const& settings);
        arc::Future<Result<JoinLevelResult>> joinLevel(std::string_view levelKey);
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

        void cancelReconnect();
    };
}