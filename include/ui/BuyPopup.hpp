#pragma once
#include <Geode/Geode.hpp>

namespace ui {
    struct Base;
}

namespace tulip::editor {
    class BuyPopup : public cocos2d::CCNode {
    public:
        cocos2d::CCNode* m_popup;
        std::string m_text;
        geode::async::TaskHolder<geode::Result<uint32_t>> m_claimListener;

        static BuyPopup* create();

        bool init();

    private:
        void update(float dt);
    };
}