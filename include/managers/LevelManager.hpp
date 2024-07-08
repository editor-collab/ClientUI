#pragma once
#include <Geode/Geode.hpp>
#include <memory>

namespace tulip::editor {
    class LevelManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        LevelManager();
        ~LevelManager();

    public:
        static LevelManager* get();
    };
}