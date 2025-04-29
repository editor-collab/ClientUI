#include <hooks/LevelBrowserLayer.hpp>
#include <hooks/ui/LevelBrowserLayer.hpp>
#include <ui/MainScene.hpp>
#include <managers/AccountManager.hpp>
#include <managers/FetchManager.hpp>
#include <managers/LevelManager.hpp>
#include <utils/CryptoHelper.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <data/LevelEntry.hpp>
#include <ui/ShareSettings.hpp>
#include <ui/ClubstepBackground.hpp>

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
		reinterpret_cast<LevelBrowserLayerUIHook*>(this)->onMyLevels(nullptr);
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

		static auto setting = [](){
			LevelSetting setting;
			setting.title = "Test Level";
			setting.users = {
				{"user1", DefaultSharingType::Viewer},
				{"user2", DefaultSharingType::Editor},
				{"user3", DefaultSharingType::Admin},
				{"user4", DefaultSharingType::Viewer},
				{"user5", DefaultSharingType::Editor},
				{"user6", DefaultSharingType::Admin}
			};
			return setting;
		}();

		auto menuButton = CCMenuItemExt::createSpriteExtra(menuSprite, [this](CCObject* sender) {
			this->onConnect();
			// auto bg = ClubstepBackground::create();
			// bg->setPosition(0, 0);
			// this->addChild(bg, 10);
			// auto shareSettings = ShareSettings::create(&setting);
		});
		menu->addChild(menuButton);
		
		menu->updateLayout();
	}
	return true;
}