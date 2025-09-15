#pragma once
#include <Geode/Geode.hpp>
#include <memory>
#include "../data/LevelEntry.hpp"

namespace tulip::editor {
    class LocalManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        LocalManager();
        ~LocalManager();

    public:
        static LocalManager* get();
    };
}