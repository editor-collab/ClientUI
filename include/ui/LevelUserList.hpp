#pragma once
#include <Geode/Geode.hpp>
#include "../data/User.hpp"
#include "../data/LevelEntry.hpp"
#include "../data/ConnectedUserList.hpp"
#include <deque>
#include <Geode/utils/async.hpp>

namespace ui {
    struct Base;
}

namespace tulip::editor {
    class LevelUserList : public cocos2d::CCNode {
    public:
        cocos2d::CCNode* m_center = nullptr;
        cocos2d::CCNode* m_popup = nullptr;
        geode::ScrollLayer* m_peopleScrollLayer = nullptr;
        cocos2d::CCNode* m_reasonPopup = nullptr;

        LevelEntry* m_entry;
        LevelSetting* m_setting;
        LevelEditorLayer* m_editorLayer;
        ConnectedUserList m_userList;
        geode::ListenerHandle m_userListHandle;
        geode::async::TaskHolder<geode::Result<>> m_kickListener;
        geode::async::TaskHolder<geode::Result<LevelEntry>> m_updateLevelListener;

        std::string m_filter = "";
        std::string m_reasonString = "";

        static LevelUserList* create(LevelEntry* entry, LevelEditorLayer* editorLayer);

        bool init(LevelEntry* entry, LevelEditorLayer* editorLayer);

    private:

        void updateUsers();

        std::vector<ConnectedUserEntry> getFilteredOnline();
        std::vector<BannedUserEntry> getFilteredBanned();

        void createReasonPopup(std::function<void(std::string_view)> callback);

        bool isUserAdminAndUp();
    };
}