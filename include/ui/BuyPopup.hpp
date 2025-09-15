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
        geode::EventListener<geode::Task<geode::Result<uint32_t>, geode::utils::web::WebProgress>> m_claimListener;

        static BuyPopup* create();

        bool init();

    private:
        void update(float dt);
    };
}