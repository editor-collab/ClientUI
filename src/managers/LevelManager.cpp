#include <managers/LevelManager.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class LevelManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    
};

LevelManager* LevelManager::get() {
    static LevelManager instance;
    return &instance;
}

LevelManager::LevelManager() : impl(std::make_unique<Impl>()) {}
LevelManager::~LevelManager() = default;