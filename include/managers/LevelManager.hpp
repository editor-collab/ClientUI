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

        Task<Result<CreateLevelResult>, WebProgress> createLevel(LevelSetting const& settings);
        Task<Result<JoinLevelResult>, WebProgress> joinLevel(std::string_view levelKey);
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

        void cancelReconnect();
    };
}