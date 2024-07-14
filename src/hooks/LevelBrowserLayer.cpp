#include <hooks/LevelBrowserLayer.hpp>
#include <ui/MainScene.hpp>
#include <managers/AccountManager.hpp>

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
				// cocos::switchToScene(MainScene::create());
				AccountManager::get()->startChallenge([=](auto result) {
					if (result.isErr()) {
						createQuickPopup(
							"Error",
							result.unwrapErr(),
							"OK",
							"Cancel",
							[](auto, auto) {}
						);
						return;
					}
					createQuickPopup(
						"Success",
						"Challenge completed!",
						"OK",
						"Cancel",
						[](auto, auto) {}
					);
				});
			});
			menu->addChild(menuButton);

			auto menu2Button = CCMenuItemExt::createSpriteExtra(menuSprite, [](CCObject* sender) {
				// cocos::switchToScene(MainScene::create());
				AccountManager::get()->authenticate([=](auto result) {
					if (result.isErr()) {
						createQuickPopup(
							"Error",
							result.unwrapErr(),
							"OK",
							"Cancel",
							[](auto, auto) {}
						);
						return;
					}
					createQuickPopup(
						"Success",
						"Logged in succesfully!",
						"OK",
						"Cancel",
						[](auto, auto) {}
					);
				});
			});
			menu->addChild(menu2Button);
			
			menu->updateLayout();
		}
		return true;
	}
};