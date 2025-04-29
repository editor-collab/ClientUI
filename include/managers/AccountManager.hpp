#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <memory>

namespace tulip::editor {
    using geode::Result;
    using geode::Task;
    using geode::utils::web::WebProgress;

    class AccountManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        AccountManager();
        ~AccountManager();

    public:
        using Callback = std::function<void(geode::Result<>)>;

        static AccountManager* get();

        void authenticate(Callback&& callback);
        void startChallenge(Callback&& callback);

        std::string getLoginToken() const;

        std::string getAuthToken() const;
        void setAuthToken(std::string_view const token);

        Task<Result<uint32_t>, WebProgress> claimKey(std::string_view key);

        bool isAuthenticated() const;
        bool isLoggedIn() const;
    };
}