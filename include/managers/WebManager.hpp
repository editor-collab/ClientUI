#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <memory>

namespace tulip::editor {
    class WebManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        WebManager();
        ~WebManager();

    public:
        static WebManager* get();

        geode::Result<> errorCallback(geode::utils::web::WebResponse* response);
        geode::utils::web::WebRequest createRequest() const;
        geode::utils::web::WebRequest createAuthenticatedRequest() const;

        std::string getServerURL() const;
        std::string getServerURL(std::string_view path) const;

        void setLoginToken(std::string token);
        std::string getLoginToken() const;
        
        inline bool isLoggedIn() const {
            return !getLoginToken().empty();
        }

        bool isSocketConnected() const;
    };
}