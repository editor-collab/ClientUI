#include <hooks/EditLevelLayer.hpp>
#include <managers/LevelManager.hpp>
#include <managers/AccountManager.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;
using namespace tulip::editor;


#include <Geode/modify/GameManager.hpp>

struct ExitHook : Modify<ExitHook, GameManager> {
	struct Fields {
		EventListener<Task<Result<>, WebProgress>> leaveLevelListener;
	};

	void returnToLastScene(GJGameLevel* level) {
		if (LevelManager::get()->isInLevel()) {
			auto task = LevelManager::get()->leaveLevel();
			m_fields->leaveLevelListener.bind([=](auto* event) {
				if (auto resultp = event->getValue(); resultp && resultp->isOk()) {
					auto token = AccountManager::get()->getLoginToken();
					DispatchEvent<std::string_view>(
						"leave-level"_spr, token
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
			m_fields->leaveLevelListener.setFilter(task);
		}

		GameManager::returnToLastScene(level);
	}
};

#include <Geode/modify/EditLevelLayer.hpp>

struct EditLevelLayerHook : Modify<EditLevelLayerHook, EditLevelLayer> {
	struct Fields {
		EventListener<Task<Result<std::pair<uint32_t, std::string>>, WebProgress>> createLevelListener;
		EventListener<Task<Result<>, WebProgress>> deleteLevelListener;
	};

    $override
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

        if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("folder-menu"))) {
			auto alternate = Mod::get()->getSettingValue<bool>("alt-button");
			auto menuSprite = CCSprite::createWithSpriteFrameName(
				alternate ? "AlternateMenuButton.png"_spr : "MenuButton.png"_spr
			);
			menuSprite->setScale(0.9f);

			auto sprite1 = CircleButtonSprite::create(CCLabelBMFont::create("Create", "bigFont.fnt"), CircleBaseColor::Cyan);

			auto menuButton = CCMenuItemExt::createSpriteExtra(sprite1, [=, this](CCObject* sender) {
				auto task = LevelManager::get()->createLevel(0, EditorIDs::getID(level));
				m_fields->createLevelListener.bind([=, this](auto* event) {
					if (auto resultp = event->getValue(); resultp && resultp->isOk()) {
						auto const clientId = resultp->unwrap().first;
						auto token = AccountManager::get()->getLoginToken();
						auto hosted = resultp->unwrap().second;
						DispatchEvent<std::string_view, uint32_t, GJGameLevel*, std::string_view>(
							"create-level"_spr, token, clientId, level, hosted
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
				m_fields->createLevelListener.setFilter(task);
			});
			menu->addChild(menuButton);

			auto sprite4 = CircleButtonSprite::create(CCLabelBMFont::create("Delete", "bigFont.fnt"), CircleBaseColor::DarkPurple);

			auto menu4Button = CCMenuItemExt::createSpriteExtra(sprite4, [this](CCObject* sender) {
				// cocos::switchToScene(MainScene::create());
				auto levels = LevelManager::get()->getHostedLevels();
				if (levels.empty()) {
					createQuickPopup(
						"Error",
						"No levels found",
						"OK",
						"Cancel",
						[](auto, auto) {}
					);
					return;
				}

				auto task = LevelManager::get()->deleteLevel(levels.back());
				m_fields->deleteLevelListener.bind([=, this](auto* event) {
					if (auto resultp = event->getValue(); resultp && resultp->isOk()) {
						auto token = AccountManager::get()->getLoginToken();
						DispatchEvent<std::string_view, uint32_t>(
							"delete-level"_spr, token, LevelManager::get()->getClientId()
						).post();
						createQuickPopup(
							"Success",
							"Level deleted",
							"OK",
							"Cancel",
							[](auto, auto) {}
						);
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
			});

			menu->addChild(menu4Button);
			
			menu->updateLayout();
		}
		return true;
	}
};