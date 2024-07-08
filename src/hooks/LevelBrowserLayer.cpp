#include <hooks/LevelBrowserLayer.hpp>
#include <ui/TulipEditorScene.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

#include <Geode/modify/LevelBrowserLayer.hpp>

struct LevelBrowserLayerHook : Modify<LevelBrowserLayerHook, LevelBrowserLayer> {
    $override
	bool init(GJSearchObject* searchObject) {
		if (!LevelBrowserLayer::init(searchObject)) return false;

		if (searchObject->m_searchType != SearchType::MyLevels) return true;

		if (auto menu = static_cast<CCMenu*>(this->getChildByID("my-levels-menu"))) {
			auto alternate = Mod::get()->getSettingValue<bool>("alt-button");
			auto menuSprite = CCSprite::createWithSpriteFrameName(
				alternate ? "AlternateMenuButton.png"_spr : "MenuButton.png"_spr
			);
			menuSprite->setScale(0.9f);

			auto menuButton = CCMenuItemExt::createSpriteExtra(menuSprite, [](CCObject* sender) {
				cocos::switchToScene(TulipEditorScene::create());
			});
			menu->addChild(menuButton);
			
			menu->updateLayout();
		}
		return true;
	}
};