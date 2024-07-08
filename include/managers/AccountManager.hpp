#pragma once
#include <Geode/Geode.hpp>
#include <memory>

namespace tulip::editor {
    class AccountManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        AccountManager();
        ~AccountManager();

    public:
        static AccountManager* get();
    };
}