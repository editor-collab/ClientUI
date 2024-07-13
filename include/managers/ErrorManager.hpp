#pragma once
#include <Geode/Geode.hpp>
#include <memory>

namespace tulip::editor {
    class ErrorManager {
        class Impl;
        std::unique_ptr<Impl> impl;

        ErrorManager();
        ~ErrorManager();

    public:
        static ErrorManager* get();
    };
}