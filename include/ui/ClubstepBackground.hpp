#pragma once
#include <Geode/Geode.hpp>
#include "../data/LevelEntry.hpp"
#include <deque>

namespace ui {
    struct Base;
}

namespace tulip::editor {
    class ClubstepBackground : public cocos2d::CCNode {
    public:
        std::vector<GameObject*> m_placedObjects;
        cocos2d::CCDrawNode* m_drawNode;
        cocos2d::CCSprite* m_bgSprite;
        cocos2d::CCDrawNode* m_gridNode;

        static ClubstepBackground* create();

        bool init();

    private:
        void update(float dt);
    };
}