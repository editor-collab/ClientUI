#pragma once
#include <Geode/Geode.hpp>
#include <memory>
#include <Geode/utils/web.hpp>
#include "../data/LevelEntry.hpp"

namespace tulip::editor {
    using geode::Result;
    using geode::Task;
    using geode::utils::web::WebProgress;

    class FetchManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        FetchManager();
        ~FetchManager();

    public:
        static FetchManager* get();

        using TaskType = Task<Result<std::vector<LevelEntry>>, WebProgress>;

        Task<Result<std::vector<LevelEntry>>, WebProgress> getMyLevels();
        Task<Result<std::vector<LevelEntry>>, WebProgress> getSharedWithMe();
        Task<Result<std::vector<LevelEntry>>, WebProgress> getDiscover();
        size_t getHostableCount() const;
        void addHostableCount(size_t count);
    };
}