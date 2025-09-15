#pragma once
#include "LevelKey.hpp"
#include <string>

namespace tulip::editor {
    class DiscoverableLevel {
    public:
        LevelKey m_levelKey = 0;
        std::string m_levelName;
        std::string m_hostName;
        int m_joinedPlayers = 0;
        bool m_isEditor = false;
    };
}