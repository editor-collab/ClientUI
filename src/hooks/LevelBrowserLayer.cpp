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
#include <managers/WebManager.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

void LevelBrowserLayerHook::refreshButton() {
	if (m_fields->menuButton) m_fields->menuButton->removeFromParent();
	std::string filename;
	if (Mod::get()->getSettingValue<bool>("alternate-button")) {
		if (!WebManager::get()->isLoggedIn()) {
			filename = "DeactiveAlternateMenuButton.png"_spr;
		}
		else {
			filename = "AlternateMenuButton.png"_spr;
		}
	}
	else {
		if (!WebManager::get()->isLoggedIn()) {
			filename = "DeactiveMenuButton.png"_spr;
		}
		else {
			filename = "MenuButton.png"_spr;
		}
	}
	
	auto menuSprite = CCSprite::createWithSpriteFrameName(filename.c_str());
	menuSprite->setScale(0.9f);

	auto menuButton = CCMenuItemExt::createSpriteExtra(menuSprite, [this](CCObject* sender) {
		if (!WebManager::get()->isLoggedIn()) {
			m_fields->loginListener.spawn(AccountManager::get()->login(argon::getGameAccountData()), [this](auto res) {
				this->onLogin(res);
			});
		}
		else {
			this->onLogout(AccountManager::get()->logout());
		}
	});
	menuButton->setScale(0.9f);

	m_fields->menuButton = menuButton;

	if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("my-levels-menu"))) {
		menu->addChild(menuButton);
		menu->updateLayout();
	}
	this->loadPage(m_searchObject);
}

void LevelBrowserLayerHook::onLogin(Result<std::string> result) {
	// auto req = WebManager::get()->createAuthenticatedRequest();
	// auto task = req.post(WebManager::get()->getServerURL("admin/generate_key"));
	// task.listen([this](web::WebResponse* response) {
	// 	auto result = response->string();
	// 	if (result.isErr()) {
	// 		Notification::create("Failed to generate key!", nullptr)->show();
	// 	}
	// 	else {
	// 		auto key = result.unwrap();
	// 		log::info("Generated key: {}", key);
	// 		Notification::create(fmt::format("Generated key: {}", key), nullptr)->show();
	// 	}
	// });

	// auto req = WebManager::get()->createAuthenticatedRequest();
	// req.param("old_account_name", "dannygd28");
	// req.param("new_account_name", "Dannyplays64");
	// m_fields->adminTask.spawn(req.post(WebManager::get()->getServerURL("admin/transfer_account")), [this](auto response) {
	// 	auto result = response.string();
	// 	if (result.isErr()) {
	// 		Notification::create(fmt::format("Failed to transfer account: {}", result.unwrapErr()), nullptr)->show();
	// 	}
	// 	else {
	// 		auto key = result.unwrap();
	// 		log::info("Transfer account: {}", key);
	// 		Notification::create(fmt::format("Transfer account: {}", key), nullptr)->show();
	// 	}
	// });

	// auto req = WebManager::get()->createAuthenticatedRequest();
	// req.param("account_id", "accountId");
	// auto task = req.post(WebManager::get()->getServerURL("admin/ban_user"));
	// task.listen([this](web::WebResponse* response) {
	// 	auto result = response->string();
	// 	if (result.isErr()) {
	// 		Notification::create("Failed to ban user!", nullptr)->show();
	// 	}
	// 	else {
	// 		auto res = result.unwrap();
	// 		log::info("Ban user: {}", res);
	// 		Notification::create(fmt::format("Ban user: {}", res), nullptr)->show();
	// 	}
	// });

	// auto req = WebManager::get()->createAuthenticatedRequest();
	// req.param("key", "claimKey");
	// auto task = req.post(WebManager::get()->getServerURL("admin/revoke_key"));
	// task.listen([this](web::WebResponse* response) {
	// 	auto result = response->string();
	// 	if (result.isErr()) {
	// 		Notification::create("Failed to revoke key!", nullptr)->show();
	// 	}
	// 	else {
	// 		auto res = result.unwrap();
	// 		log::info("Revoke key: {}", res);
	// 		Notification::create(fmt::format("Revoke key: {}", res), nullptr)->show();
	// 	}
	// });

	if (result.isErr()) {
		log::info("Failed to login: {}", result.unwrapErr());
		Notification::create(fmt::format("Failed to login!: {}", result.unwrapErr()), nullptr)->show();
	}
	else {
		Notification::create("Logged in successfully!", nullptr)->show();
		LevelBrowserLayerUIHook::Fields::initialCall = false;

		WebManager::get()->setLoginToken(result.unwrap());

		if (Mod::get()->getSavedValue<bool>("shown-globed-compatibility-popup") == false) {
			geode::createQuickPopup(
				"Editor Collab", 
				"<cg>Editor Collab</c> is now compatible with <co>Globed</c>! "
				"You can <cc>see your friends</c> playtesting <cl>while editing</c> at the same time, just <cb>join the same server</c>!",
				"OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, true
			);
			Mod::get()->setSavedValue("shown-globed-compatibility-popup", true);
		}

		LevelBrowserLayerUIHook::from(this)->onLocalLevels(nullptr);
		this->refreshButton();
	}
}

void LevelBrowserLayerHook::onLogout(Result<> result) {
	if (result.isErr()) {
		Notification::create("Failed to logout!", nullptr)->show();
	}
	else {
		WebManager::get()->setLoginToken("");

		Notification::create("Logged out successfully!", nullptr)->show();
	}
	this->refreshButton();
}

$override
bool LevelBrowserLayerHook::init(GJSearchObject* searchObject) {
	if (!LevelBrowserLayer::init(searchObject)) return false;

	Fields::self = this;

	if (searchObject->m_searchType != SearchType::MyLevels) return true;

	if (Mod::get()->getSavedValue<bool>("shown-introduction-popup") == false) {
		auto popup = geode::createQuickPopup(
			"Editor Collab", 
			"Press the <cl>button</c> at the <cp>bottom left corner</c> to login to <cg>Editor Collab</c> and <co>share your levels</c> with others!", 
			"OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, false);
		popup->m_scene = this;
		popup->show();
		Mod::get()->setSavedValue("shown-introduction-popup", true);
	}

	if (!Loader::get()->isModLoaded("alk.editor-collab")) {
		log::warn("Editor Collab mod is not loaded, Editor Collab UI requires the mod to function.");
		auto popup = geode::createQuickPopup(
			"Editor Collab (Error)", 
			"<cg>Editor Collab</c> mod <cr>is not loaded</c>, <cf>Editor Collab UI</c> requires the mod to function.", 
			"OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {
				if (auto task = geode::openInfoPopup("alk.editor-collab")) {
					m_fields->modPageTask.spawn(std::move(*task), [=](auto res) {

					});
				}
			}, false);
		popup->m_scene = this;
		popup->show();
		// return true;
	}

	this->refreshButton();
	return true;
}