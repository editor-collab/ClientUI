#include <managers/AccountManager.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class AccountManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;


};

AccountManager* AccountManager::get() {
    static AccountManager instance;
    return &instance;
}

AccountManager::AccountManager() : impl(std::make_unique<Impl>()) {}
AccountManager::~AccountManager() = default;