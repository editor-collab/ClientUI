#pragma once
#include <Geode/Geode.hpp>
#include "../data/LevelEntry.hpp"
#include <memory>

namespace tulip::editor {
    class CellManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        CellManager();
        ~CellManager();

    public:

        static CellManager* get();

        void applyMyLevel(LevelCell* cell, LevelEntry const& entry);
        void applySharedLevel(LevelCell* cell, LevelEntry const& entry);
        void applyDiscoverLevel(LevelCell* cell, LevelEntry const& entry);
    };
}