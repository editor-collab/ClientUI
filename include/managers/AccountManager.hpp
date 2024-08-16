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
        using Callback = geode::utils::MiniFunction<void(geode::Result<>)>;

        static AccountManager* get();

        void authenticate(Callback&& callback);
        void startChallenge(Callback&& callback);

        std::string getLoginToken() const;
    };
}