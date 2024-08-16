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

        geode::utils::web::WebRequest createRequest() const;
        geode::utils::web::WebRequest createAuthenticatedRequest() const;

        std::string getServerURL() const;
        std::string getServerURL(std::string_view path) const;
    };
}