#pragma once
#include <Geode/Geode.hpp>
#include <memory>
#include <Geode/utils/web.hpp>

namespace tulip::editor {
    using geode::Result;
    using geode::utils::MiniFunction;
    using geode::Task;
    using geode::utils::web::WebProgress;
    class LevelManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        LevelManager();
        ~LevelManager();

    public:
        static LevelManager* get();

        Task<Result<std::pair<uint32_t, std::string>>, WebProgress> createLevel(int slotId, int uniqueId);
        Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, WebProgress> joinLevel(std::string_view levelKey);
        Task<Result<>, WebProgress> leaveLevel();
        Task<Result<>, WebProgress> deleteLevel(std::string_view levelKey);
        Task<Result<std::vector<uint8_t>>, WebProgress> getSnapshot(std::string_view levelKey, std::string_view hash);

        std::vector<std::string> getHostedLevels() const;

        uint32_t getClientId() const;

        bool isInLevel() const;
    };
}