#pragma once
#include <Geode/Geode.hpp>
#include <cstdint>
#include "../data/LevelEntry.hpp"
#include "../data/ConnectedUserList.hpp"

namespace tulip::editor {
	class EditorOverlay : public cocos2d::CCDrawNode {
	public:
		std::unordered_map<uint32_t, cocos2d::CCRect> m_selectionRects;

		std::unordered_map<uint32_t, geode::Ref<cocos2d::CCLabelBMFont>> m_userLabels;
		std::unordered_map<uint32_t, geode::Ref<cocos2d::extension::CCScale9Sprite>> m_userLabelsShadow;

		geode::Ref<cocos2d::CCNode> m_userListListener;
		geode::Ref<cocos2d::CCNode> m_redrawListener;
		geode::Ref<cocos2d::CCNode> m_drawListener;
		geode::Ref<cocos2d::CCNode> m_levelSettingListener;

		LevelSetting m_levelSetting;
		ConnectedUserList m_userList;
		geode::Ref<cocos2d::CCDrawNode> m_positionSliderIndicator = nullptr;
		geode::Ref<Slider> m_slider = nullptr;
		uint32_t m_hostAccountId;

		EditorOverlay();
		~EditorOverlay();
		static EditorOverlay* create(uint32_t hostAccountId);
		bool init(uint32_t hostAccountId);

		cocos2d::ccColor3B getClientColor(uint32_t clientLevelId);

		bool shouldDrawFor(uint32_t selectedId);

		static EditorOverlay* get();
		void reset();

		void drawSelectionRect(uint32_t clientLevelId, cocos2d::CCRect rectangle);
		void redrawSelectionRect();
		void recalculateSelectionRect();
	};
}
