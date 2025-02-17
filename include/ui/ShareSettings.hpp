#pragma once
#include <Geode/Geode.hpp>
#include "../data/LevelEntry.hpp"
#include <deque>

namespace ui {
    struct Base;
}

namespace tulip::editor {
    class ShareSettings : public cocos2d::CCNode {
    public:
        cocos2d::CCNode* m_center = nullptr;

        LevelEntry* m_entry;
        LevelSetting* m_setting;
        GJGameLevel* m_realLevel;
        LevelEditorLayer* m_editorLayer;
        cocos2d::CCNode* m_list = nullptr;
        cocos2d::CCNode* m_popup = nullptr;
        geode::SimpleTextArea* m_generalAccessText = nullptr;
        geode::SimpleTextArea* m_generalAccessDescription = nullptr;
        geode::SimpleTextArea* m_generalAccessTypeText = nullptr;
        geode::TextInput* m_shareWithInput = nullptr;
        cocos2d::CCNode* m_generalAccessTypeRow = nullptr;
        geode::ScrollLayer* m_peopleScrollLayer = nullptr;
        geode::SimpleTextArea* m_shareDescription = nullptr;

        bool m_resetScroll = false;

        static ShareSettings* create(LevelEntry* setting, LevelEditorLayer* editorLayer);

        bool init(LevelEntry* setting, LevelEditorLayer* editorLayer);

    private:
        ui::Base* generatePersonEntry(std::string name, DefaultSharingType type);

        std::string getSharingTypeString(DefaultSharingType type);

        void updateValues();

        void addSharedUser(cocos2d::CCObject* sender);

        void changeGeneralAccess(cocos2d::CCObject* sender);

        void changePersonEntryLeft(cocos2d::CCObject* sender);
        void changePersonEntryRight(cocos2d::CCObject* sender);

        void startSharing(cocos2d::CCObject* sender);
        void stopSharing(cocos2d::CCObject* sender);
    };
}