#include <hooks/LevelBrowserLayer.hpp>
#include <ui/MainScene.hpp>
#include <managers/AccountManager.hpp>
#include <managers/LevelManager.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <utils/CryptoHelper.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

#include <Geode/modify/LevelBrowserLayer.hpp>

struct LevelBrowserLayerHook : Modify<LevelBrowserLayerHook, LevelBrowserLayer> {
	struct Fields {
		EventListener<Task<Result<std::pair<uint32_t, std::vector<uint8_t>>>, WebProgress>> joinLevelListener;
	};

    $override
	bool init(GJSearchObject* searchObject) {
		if (!LevelBrowserLayer::init(searchObject)) return false;

		if (searchObject->m_searchType != SearchType::MyLevels) return true;

		if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("my-levels-menu"))) {
			auto alternate = Mod::get()->getSettingValue<bool>("alt-button");
			auto menuSprite = CCSprite::createWithSpriteFrameName(
				alternate ? "AlternateMenuButton.png"_spr : "MenuButton.png"_spr
			);
			menuSprite->setScale(0.9f);

			auto sprite1 = CircleButtonSprite::create(CCLabelBMFont::create("Challenge", "bigFont.fnt"), CircleBaseColor::Pink);

			auto menuButton = CCMenuItemExt::createSpriteExtra(sprite1, [](CCObject* sender) {
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

			auto sprite2 = CircleButtonSprite::create(CCLabelBMFont::create("Login", "bigFont.fnt"), CircleBaseColor::Gray);

			auto menu2Button = CCMenuItemExt::createSpriteExtra(sprite2, [](CCObject* sender) {
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

			auto sprite3 = CircleButtonSprite::create(CCLabelBMFont::create("Join", "bigFont.fnt"), CircleBaseColor::DarkPurple);
			auto menu3Button = CCMenuItemExt::createSpriteExtra(sprite3, [=, this](CCObject* sender) {
				auto task = LevelManager::get()->joinLevel("testtest"/*levels.back()*/);
				m_fields->joinLevelListener.bind([=, this](auto* event) {
					if (auto resultp = event->getValue(); resultp && resultp->isOk()) {
						auto const clientId = resultp->unwrap().first;
						auto token = AccountManager::get()->getLoginToken();
						DispatchEvent<std::string_view, uint32_t, std::string_view, std::vector<uint8_t> const*>(
							"join-level"_spr, token, clientId, "testtest"/*hosted*/, &resultp->unwrap().second
						).post();
					}
					else if (resultp) {
						createQuickPopup(
							"Error",
							resultp->unwrapErr(),
							"OK",
							"Cancel",
							[](auto, auto) {}
						);
					}
				});
				m_fields->joinLevelListener.setFilter(task);
			});
			menu->addChild(menu3Button);
			
			menu->updateLayout();
		}
		return true;
	}
};