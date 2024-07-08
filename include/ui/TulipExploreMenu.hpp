#pragma once
#include <Geode/Geode.hpp>

namespace tulip::editor {
    class GenericList;

    class TulipExploreMenu : public cocos2d::CCNode {
    public:
        GenericList* m_list;
        geode::Ref<cocos2d::CCLabelBMFont> m_codeLabel;
        geode::Ref<cocos2d::CCMenu> m_menu;
        geode::Ref<geode::TextInput> m_textInput;
        bool m_isExplore;

        static TulipExploreMenu* create(cocos2d::CCSize const& size, bool isExplore);
        bool init(cocos2d::CCSize const& size, bool isExplore);

        void onJoinLevel(cocos2d::CCObject* sender);
        void onJoinCode(cocos2d::CCObject* sender);
        void onPromo(cocos2d::CCObject* sender);
        void onJoinHosted(cocos2d::CCObject* sender);
        void onOpenBrowser(cocos2d::CCObject* sender);
        void onDeleteLast(cocos2d::CCObject* sender);

        void createBanner(cocos2d::CCSize const& size);
        void createPromoBanner(cocos2d::CCSize const& size);
        void createHostBanner(cocos2d::CCSize const& size);
        void createExploreBanner(cocos2d::CCSize const& size);
        void createCreateBanner(cocos2d::CCSize const& size);
        void createList(cocos2d::CCSize const& size, bool isExplore);
    };
}