#pragma once
#include <Geode/Geode.hpp>
#include "../data/LevelEntry.hpp"
#include <deque>
#include <Geode/utils/async.hpp>

namespace ui {
    struct Base;
}

namespace tulip::editor {
    class LimitsSettings : public cocos2d::CCNode {
    public:
        SettingUserEntry* m_entry;
        LevelSetting* m_setting;
        std::string m_key;
        geode::async::TaskHolder<geode::Result<LevelEntry>> m_updateLevelListener;

        cocos2d::CCNode* m_popup = nullptr;

        geode::TextInput* m_minRangeInput = nullptr;
        geode::TextInput* m_maxRangeInput = nullptr;

        cocos2d::CCNode* m_layersGrid = nullptr;
        cocos2d::CCNode* m_groupsGrid = nullptr;
        cocos2d::CCNode* m_colorsGrid = nullptr;
        cocos2d::CCNode* m_itemsGrid = nullptr;

        static LimitsSettings* create(std::string_view key, LevelSetting* setting, SettingUserEntry* entry);

        bool init(std::string_view key, LevelSetting* setting, SettingUserEntry* entry);

    private:
        ui::Base* generateGrid(cocos2d::CCNode** node, std::string name, std::string id, std::vector<AllowedRange>* range);

        ui::Base* generateItem(size_t idx, std::string id, std::vector<AllowedRange>* range);

        ui::Base* generateInput(geode::TextInput** input, std::string name, std::string id);

        void updateValues();
    };
}