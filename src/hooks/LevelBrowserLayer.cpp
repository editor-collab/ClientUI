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
		if (!AccountManager::get()->isLoggedIn()) {
			filename = "DeactiveAlternateMenuButton.png"_spr;
		}
		else {
			filename = "AlternateMenuButton.png"_spr;
		}
	}
	else {
		if (!AccountManager::get()->isLoggedIn()) {
			filename = "DeactiveMenuButton.png"_spr;
		}
		else {
			filename = "MenuButton.png"_spr;
		}
	}
	
	auto menuSprite = CCSprite::createWithSpriteFrameName(filename.c_str());
	menuSprite->setScale(0.9f);

	auto menuButton = CCMenuItemExt::createSpriteExtra(menuSprite, [this](CCObject* sender) {
		if (!AccountManager::get()->isLoggedIn()) {
			AccountManager::get()->authenticate(std::bind(&LevelBrowserLayerHook::onLogin, this, std::placeholders::_1));
		}
		else {
			AccountManager::get()->logout(std::bind(&LevelBrowserLayerHook::onLogout, this, std::placeholders::_1));
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

void LevelBrowserLayerHook::onLogin(Result<> result) {
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
	// req.param("old_account_name", "oldAccount");
	// req.param("new_account_name", "newAccount");
	// auto task = req.post(WebManager::get()->getServerURL("admin/transfer_account"));
	// task.listen([this](web::WebResponse* response) {
	// 	auto result = response->string();
	// 	if (result.isErr()) {
	// 		Notification::create("Failed to transfer account!", nullptr)->show();
	// 	}
	// 	else {
	// 		auto key = result.unwrap();
	// 		log::info("Transfer account: {}", key);
	// 		Notification::create(fmt::format("Transfer account: {}", key), nullptr)->show();
	// 	}
	// });

	if (result.isErr()) {
		log::info("Failed to login: {}", result.unwrapErr());
		Notification::create(fmt::format("Failed to login!: {}", result.unwrapErr()), nullptr)->show();
	}
	else {
		Notification::create("Logged in successfully!", nullptr)->show();
		LevelBrowserLayerUIHook::Fields::initialCall = false;

		if (Mod::get()->getSavedValue<bool>("shown-globed-compatibility-popup") == false) {
			geode::createQuickPopup(
				"Editor Collab", 
				"<cg>Editor Collab</c> is now compatible with <co>Globed</c>! "
				"You can <cc>see your friends</c> playtesting <cl>while editing</c> at the same time, just <cb>join the same server</c>!",
				"OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, true
			);
			Mod::get()->setSavedValue("shown-globed-compatibility-popup", true);
		}

		if (Fields::self == this) {
			LevelBrowserLayerUIHook::from(this)->onLocalLevels(nullptr);
			this->refreshButton();
		}
	}
}

void LevelBrowserLayerHook::onLogout(Result<> result) {
	if (result.isErr()) {
		Notification::create("Failed to logout!", nullptr)->show();
	}
	else {
		Notification::create("Logged out successfully!", nullptr)->show();
	}
	if (Fields::self == this) {
		this->refreshButton();
	}
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
				m_fields->modPageTask = geode::openInfoPopup("alk.editor-collab");
			}, false);
		popup->m_scene = this;
		popup->show();
		return true;
	}

	this->refreshButton();
	return true;
}