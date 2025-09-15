#include <managers/ErrorManager.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class ErrorManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    
};

ErrorManager* ErrorManager::get() {
    static ErrorManager instance;
    return &instance;
}

ErrorManager::ErrorManager() : impl(std::make_unique<Impl>()) {}
ErrorManager::~ErrorManager() = default;