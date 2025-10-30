#include <ui/EditorOverlay.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <managers/BrowserManager.hpp>
#include <hooks/ui/LevelEditorLayer.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

EditorOverlay::EditorOverlay() :
	m_selectionRects(),
	m_userLabels(),
	m_userLabelsShadow() {}

EditorOverlay::~EditorOverlay() {

}

EditorOverlay* EditorOverlay::get() {
	if (!LevelEditorLayer::get()) {
		return nullptr;
	}
	return static_cast<LevelEditorLayerUIHook*>(LevelEditorLayer::get())->m_fields->editorOverlay;
}

void EditorOverlay::reset() {
	m_selectionRects.clear();
}

EditorOverlay* EditorOverlay::create(uint32_t hostAccountId) {
	EditorOverlay* ret = new (std::nothrow) EditorOverlay;
	if (ret && ret->init(hostAccountId)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool EditorOverlay::init(uint32_t hostAccountId) {
	if (!CCDrawNode::init()) {
		return false;
	}
	m_hostAccountId = hostAccountId;

	DispatchEvent<ConnectedUserList*>("get-user-list"_spr, &m_userList).post();

    m_userListListener = EventListenerNode<DispatchFilter<ConnectedUserList>>::create(
        EventListener([this](ConnectedUserList userList) {
            m_userList = userList;
			this->redrawSelectionRect();
            return ListenerResult::Propagate;
        }, DispatchFilter<ConnectedUserList>("alk.editor-collab/update-user-list"))
    );

	m_drawListener = EventListenerNode<DispatchFilter<uint32_t, CCRect>>::create(
		EventListener([this](uint32_t clientLevelId, CCRect rectangle) {
			this->drawSelectionRect(clientLevelId, rectangle);
			return ListenerResult::Propagate;
		}, DispatchFilter<uint32_t, CCRect>("alk.editor-collab/draw-selection-overlay"))
	);

	m_redrawListener = EventListenerNode<DispatchFilter<>>::create(
		EventListener([this]() {
			this->redrawSelectionRect();
			return ListenerResult::Propagate;
		}, DispatchFilter<>("alk.editor-collab/redraw-selection-overlay"))
	);

	m_levelSettingListener = EventListenerNode<DispatchFilter<LevelSetting>>::create(
		EventListener([this](LevelSetting levelSetting) {
			m_levelSetting = levelSetting;
			// God i cant wait to change this all
			LevelEntry* entry = BrowserManager::get()->getLevelEntry(LevelEditorLayer::get()->m_level);
			entry->settings = m_levelSetting;
			this->redrawSelectionRect();
			return ListenerResult::Propagate;
		}, DispatchFilter<LevelSetting>("alk.editor-collab/update-level-setting"))
	);

	if (auto slider = EditorUI::get()->getChildByID("position-slider")) {
		m_positionSliderIndicator = CCDrawNode::create();
		slider->addChild(m_positionSliderIndicator, -1);
		m_slider = static_cast<Slider*>(slider);
	}

	return true;
}

cocos2d::ccColor3B EditorOverlay::getClientColor(uint32_t clientLevelId) {
	static std::array<ccColor3B, 12> colors = {
		ccc3(255, 127, 255), ccc3(127, 255, 255), ccc3(255, 255, 127), ccc3(255, 127, 127),
		ccc3(127, 127, 255), ccc3(127, 255, 127), ccc3(255, 0, 127),   ccc3(0, 127, 255),
		ccc3(127, 255, 0),   ccc3(255, 127, 0),   ccc3(127, 0, 255),   ccc3(0, 255, 127)
	};
	return colors[clientLevelId % colors.size()];
}

#include <Geode/modify/EditorUI.hpp>
class $modify(EditorUI) {
    void updateZoom(float zoom) {
        EditorUI::updateZoom(zoom);
		if (EditorOverlay::get() == nullptr) return;
        EditorOverlay::get()->redrawSelectionRect();
    }
};

bool EditorOverlay::shouldDrawFor(uint32_t selectedId) {
	for (auto& user : m_userList.users) {
		if (user.clientLevelId == selectedId) {
			if (user.user.accountId == GJAccountManager::get()->m_accountID) {
				return false;
			}
			return true;
		}
	}
	return false;
}

void EditorOverlay::drawSelectionRect(uint32_t clientLevelId, CCRect rectangle) {
	m_selectionRects[clientLevelId] = rectangle;
	this->redrawSelectionRect();
}

void EditorOverlay::redrawSelectionRect() {
	if (!EditorUI::get() || !LevelEditorLayer::get()) return;

	this->clear();
	if (m_positionSliderIndicator) m_positionSliderIndicator->clear();
	for (auto [id, label] : m_userLabels) {
		label->removeFromParent();
	}
	for (auto [id, shadow] : m_userLabelsShadow) {
		shadow->removeFromParent();
	}

	for (auto& [selectedId, rect] : m_selectionRects) {
		if (rect.equals(CCRectZero)) {
			continue;
		}

		if (!this->shouldDrawFor(selectedId)) {
			continue;
		}

		std::array<CCPoint, 4> points;
		points[0] = CCPointMake(rect.getMinX() - 5, rect.getMinY() - 5);
		points[1] = CCPointMake(rect.getMinX() - 5, rect.getMaxY() + 5);
		points[2] = CCPointMake(rect.getMaxX() + 5, rect.getMaxY() + 5);
		points[3] = CCPointMake(rect.getMaxX() + 5, rect.getMinY() - 5);

		auto color = ccc4FFromccc3B(this->getClientColor(selectedId));
		auto colorA = ccc4FFromccc4B(ccc4(0, 0, 0, 0));
		auto black = ccc4FFromccc4B(ccc4(0, 0, 0, 100));
		auto thickness = .5f;

		auto editorScale = LevelEditorLayer::get()->m_objectLayer->getScale();
		auto unscaledMult = std::clamp(rect.size.width / 30.0f, 0.7f, 1.5f);
		auto multiplier = std::clamp(unscaledMult * std::sqrtf(editorScale), 0.5f, 2.0f) / editorScale;
		auto labelPosition = CCPointMake(rect.getMidX(), rect.getMaxY() + 5 + 20 * multiplier);

		if (
			m_levelSetting.hideUsers == false || 
			m_hostAccountId == GJAccountManager::get()->m_accountID ||
			m_levelSetting.getUserType(GJAccountManager::get()->m_username) == DefaultSharingType::Admin
		) {
			auto name = m_userList.contains(selectedId) ? m_userList.find(selectedId)->user.accountName : fmt::format("User {}", int(selectedId));
			auto label = CCLabelBMFont::create(
				name.c_str(), "chatFont.fnt"
			);
			label->setScale(1.f * multiplier);
			label->setColor(this->getClientColor(selectedId));
			label->setAnchorPoint(ccp(0.5, 0.5));
			label->setPosition(labelPosition);
			m_userLabels[selectedId] = label;

			auto shadow = CCScale9Sprite::create("square02_001.png");
			shadow->setContentSize((label->getContentSize() + ccp(10, 6)) / 0.3f);
			shadow->setPosition(labelPosition);
			shadow->setOpacity(100);
			shadow->setColor(ccc3(0, 0, 0));
			shadow->setScale(1.f * multiplier * 0.3f);
			shadow->setAnchorPoint(ccp(0.5, 0.5));
			m_userLabelsShadow[selectedId] = shadow;

			this->addChild(shadow);
			this->addChild(label);
		}

		this->drawPolygon(
			const_cast<CCPoint*>(points.data()), points.size(), colorA, thickness + .5f * multiplier, black
		);

		this->drawPolygon(
			const_cast<CCPoint*>(points.data()), points.size(), colorA, thickness, color
		);


		if (m_slider) {
			auto length = m_slider->getThumb()->m_length - 20.f;

			auto valueFromXPos = [](float xPos) {
				auto sectionCount = std::max<int>(LevelEditorLayer::get()->m_sections.size(), 100);
				// TODO: use node ids when 1.8.2 update is out
				// auto batchLayer = getChildOfType<CCLayer>(getEditorLayer()->getChildByIDRecursive("main-node"), 0);

				auto calculated = xPos / (sectionCount * 100.0f);

				return std::clamp(calculated, 0.0f, 1.0f);
			};

			auto left = valueFromXPos(rect.getMinX());
			auto right = valueFromXPos(rect.getMaxX());

			auto sliderLeft = left * length - length / 2;
			auto sliderRight = right * length - length / 2;

			auto points = std::array<CCPoint, 4>{
				CCPointMake(sliderLeft, -2),
				CCPointMake(sliderLeft, 2),
				CCPointMake(sliderRight, 2),
				CCPointMake(sliderRight, -2),
			};

			m_positionSliderIndicator->drawPolygon(const_cast<CCPoint*>(points.data()), points.size(), color, 2, color);
		}
	}
}
