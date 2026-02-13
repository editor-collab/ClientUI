#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <argon/argon.hpp>
#include <memory>

namespace tulip::editor {
    using geode::Result;

    class AccountManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        AccountManager();
        ~AccountManager();

    public:
        static AccountManager* get();

        arc::Future<Result<std::string>> login(argon::AccountData accountData);
        Result<> logout();

        arc::Future<Result<uint32_t>> claimKey(std::string_view key);
    };
}