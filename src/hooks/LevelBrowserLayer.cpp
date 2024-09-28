#include <hooks/LevelBrowserLayer.hpp>
#include <ui/MainScene.hpp>
#include <managers/AccountManager.hpp>
#include <managers/LevelManager.hpp>
#include <utils/CryptoHelper.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <data/LevelEntry.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

void LevelBrowserLayerHook::onChallenge(Result<> result) {
	if (result.isErr()) {
		Notification::create("Failed to complete the challenge!", nullptr)->show();
	}
	else {
		AccountManager::get()->authenticate(std::bind(&LevelBrowserLayerHook::onLogin, this, std::placeholders::_1, false));
	}
}

void LevelBrowserLayerHook::onLogin(Result<> result, bool challenge) {
	if (result.isErr()) {
		if (challenge) AccountManager::get()->startChallenge(std::bind(&LevelBrowserLayerHook::onChallenge, this, std::placeholders::_1));
		else Notification::create("Failed to login!", nullptr)->show();
	}
	else {
		Notification::create("Logged in successfully!", nullptr)->show();
	}
}

void LevelBrowserLayerHook::onConnect() {
	if (AccountManager::get()->getAuthToken().empty()) {
		AccountManager::get()->startChallenge(std::bind(&LevelBrowserLayerHook::onChallenge, this, std::placeholders::_1));
	}
	else {
		AccountManager::get()->authenticate(std::bind(&LevelBrowserLayerHook::onLogin, this, std::placeholders::_1, true));
	}
}

$override
bool LevelBrowserLayerHook::init(GJSearchObject* searchObject) {
	if (!LevelBrowserLayer::init(searchObject)) return false;

	if (searchObject->m_searchType != SearchType::MyLevels) return true;

	if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("my-levels-menu"))) {
		auto alternate = Mod::get()->getSettingValue<bool>("alt-button");
		auto menuSprite = CCSprite::createWithSpriteFrameName(
			alternate ? "AlternateMenuButton.png"_spr : "MenuButton.png"_spr
		);
		menuSprite->setScale(0.9f);

		auto menuButton = CCMenuItemExt::createSpriteExtra(menuSprite, [this](CCObject* sender) {
			this->onConnect();
		});
		menu->addChild(menuButton);

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