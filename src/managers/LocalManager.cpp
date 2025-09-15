#include <managers/LocalManager.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class LocalManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;
};

LocalManager* LocalManager::get() {
    static LocalManager instance;
    return &instance;
}

LocalManager::LocalManager() : impl(std::make_unique<Impl>()) {}
LocalManager::~LocalManager() = default;
