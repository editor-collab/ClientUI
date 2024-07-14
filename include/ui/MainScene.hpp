#pragma once
#include <Geode/Geode.hpp>

namespace tulip::editor {
    class ExploreMenu;
    class MainScene : public cocos2d::CCLayerColor {
    public:
        cocos2d::CCSprite* m_background;
        cocos2d::CCSprite* m_listTop1;
        cocos2d::CCSprite* m_listTop2;
        ExploreMenu* m_explore;
        bool m_isExplore = false;

        MainScene();
        ~MainScene();
        
        static cocos2d::CCScene* scene();
        static MainScene* create();

        void keyBackClicked() override;

        bool init() override;
        void update(float dt) override;

        void onTab(cocos2d::CCObject* sender);
        void onAccount(cocos2d::CCObject* sender);
        void onDiscord(cocos2d::CCObject* sender);
        void onClose(cocos2d::CCObject* sender);

        void recreateMenu();
    };
}