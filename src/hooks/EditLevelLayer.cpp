#include <hooks/EditLevelLayer.hpp>
#include <managers/LevelManager.hpp>
#include <managers/WebManager.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/WebManager.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;


#include <Geode/modify/GameManager.hpp>

struct ExitHook : Modify<ExitHook, GameManager> {
	struct Fields {
		async::TaskHolder<Result<>> leaveLevelListener;
	};

	void returnToLastScene(GJGameLevel* level) {
		if (LevelManager::get()->isInLevel()) {
			CameraValue camera;
			if (auto editorLayer = LevelEditorLayer::get()) {
				camera.x = editorLayer->m_objectLayer->getPositionX();
				camera.y = editorLayer->m_objectLayer->getPositionY();
				camera.zoom = editorLayer->m_objectLayer->getScale();
			}

			m_fields->leaveLevelListener.spawn(
				LevelManager::get()->leaveLevel(camera),
				[=](auto res) {
					if (res.isOk()) {
						Dispatch<std::string_view>("leave-level"_spr).send(WebManager::get()->getLoginToken());
					}
					else {
						geode::createQuickPopup(
							"Editor Collab (Error)",
							res.unwrapErr(),
							"OK",
							nullptr,
							[](auto, auto) {},
							true
						);
					}
				}
			);
		}	
		GameManager::returnToLastScene(level);
	}
};

#include <Geode/modify/EditLevelLayer.hpp>

struct EditLevelLayerHook : Modify<EditLevelLayerHook, EditLevelLayer> {
	struct Fields {
		bool m_textChanged = false;
		
		CCMenuItemSpriteExtra* m_joinButton = nullptr;
		CCSprite* m_joinButtonSprite = nullptr;
		CCMenuItemSpriteExtra* m_deleteButton = nullptr;
		CCSprite* m_deleteButtonSprite = nullptr;
		Ref<Notification> m_notification = nullptr;


		TaskHolder<Result<LevelManager::JoinLevelResult>> m_joinListener;
		TaskHolder<Result<LevelEntry>> m_updateLevelListener;
		TaskHolder<Result<>> m_deleteLevelListener;
		// EventListener<Task<Result<LevelManager::JoinLevelResult>, WebProgress>> m_joinListener;
		ListenerHandle m_disconnectHandle;
		ListenerHandle m_levelKickedHandle;
		// EventListener<DispatchFilter<>> m_disconnectListener = DispatchFilter<>("alk.editor-collab/socket-disconnected");
		// EventListener<DispatchFilter<std::string_view>> m_levelKickedListener = DispatchFilter<std::string_view>("alk.editor-collab/level-kicked");

		~Fields() {
			if (m_notification) {
				m_notification->cancel();
			}
		}
	};	

	$override
	void textChanged(CCTextInputNode* input) {
		EditLevelLayer::textChanged(input);
		if (WebManager::get()->isLoggedIn() && BrowserManager::get()->isMyLevel(m_level)) {
			m_fields->m_textChanged = true;
			auto* entry = BrowserManager::get()->getLevelEntry(m_level);
			if (input->getTag() == 1) {
				entry->settings.title = input->getString();
			}
			else if (input->getTag() == 2) {
				entry->settings.description = input->getString();
			}
			BrowserManager::get()->setLevelValues(m_level, *entry);
		}
	}

	$override
	void onBack(CCObject* sender) {
		if (m_fields->m_textChanged) {
			if (BrowserManager::get()->isMyLevel(m_level)) {
				auto* entry = BrowserManager::get()->getLevelEntry(m_level);
				BrowserManager::get()->updateLevelEntry(m_level);

				m_fields->m_updateLevelListener.spawn(LevelManager::get()->updateLevelSettings(entry->key, entry->settings), [](auto res) {});
			}
		}

		if (LevelManager::get()->getJoinedLevelKey().has_value()) {
			geode::createQuickPopup(
				"Editor Collab",
				"Are you sure you want to cancel the connection?",
				"Cancel", "OK", [=, this](FLAlertLayer* layer, bool btn2) {
					if (btn2) {
						LevelManager::get()->cancelReconnect();
						if (m_fields->m_notification) {
							m_fields->m_notification->cancel();
							m_fields->m_notification = nullptr;
						}

						EditLevelLayer::onBack(sender);
					}
				}, true
			);
			return;
		}

		EditLevelLayer::onBack(sender);
	}

	void joinLevel(std::string levelKey) {
		m_fields->m_joinListener.spawn(LevelManager::get()->joinLevel(levelKey), [=, this](auto event) {
			if (GEODE_UNWRAP_EITHER(value, err, event)) {
				//////// log::debug("join level task succeed");
				m_fields->m_notification->runAction(CCSequence::create(
					CCDelayTime::create(0.0),
					CCCallFunc::create(m_fields->m_notification, callfunc_selector(Notification::hide)),
					nullptr
				));
				m_fields->m_notification = nullptr;

				auto token = WebManager::get()->getLoginToken();
				Dispatch<std::string_view, uint32_t, std::string_view, std::vector<uint8_t> const*, std::optional<CameraValue>, GJGameLevel*>(
					"join-level"_spr).send(token, value.clientId, levelKey, &value.snapshot, value.camera, m_level);
			}
			else {
				//////// log::debug("join level task error");
				geode::createQuickPopup("Editor Collab (Error)", err, "OK", nullptr, [](auto, auto) {}, true);
			}
		});
	}

	void updateToGray() {
		if (m_fields->m_joinButton) m_fields->m_joinButton->setEnabled(false);
		if (m_fields->m_joinButtonSprite) m_fields->m_joinButtonSprite->setColor(ccColor3B { 100, 100, 100 });
		if (m_fields->m_deleteButton) m_fields->m_deleteButton->setEnabled(false);
		if (m_fields->m_deleteButtonSprite) m_fields->m_deleteButtonSprite->setColor(ccColor3B { 100, 100, 100 });
	}

	void updateToNormal() {
		if (m_fields->m_joinButton) m_fields->m_joinButton->setEnabled(true);
		if (m_fields->m_joinButtonSprite) m_fields->m_joinButtonSprite->setColor(ccColor3B { 255, 255, 255 });
		if (m_fields->m_deleteButton) m_fields->m_deleteButton->setEnabled(true);
		if (m_fields->m_deleteButtonSprite) m_fields->m_deleteButtonSprite->setColor(ccColor3B { 255, 255, 255 });
	}

    $override
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

		m_fields->m_textChanged = false;

		if (!WebManager::get()->isLoggedIn()) return true;

		if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("level-edit-menu"))) {
			if (auto entry = BrowserManager::get()->getOnlineEntry(m_level)) {

				if (!Mod::get()->getSavedValue<bool>("shown-edit-level-tutorial")) {
					geode::createQuickPopup(
						"Editor Collab", 
						"Here you can <cy>join the level</c>! If you have <cl>Editor</c> or <cf>Admin</c> permissions, you can <co>edit</c> the <cb>level name and description</c> too.",
						"OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, true
					);
					Mod::get()->setSavedValue("shown-edit-level-tutorial", true);
				}

				for (auto child : CCArrayExt<CCNode*>(menu->getChildren())) {
					child->setVisible(false);
				}

				auto username = GJAccountManager::get()->m_username;
				auto accountId = GJAccountManager::get()->m_accountID;
				if (accountId != entry->hostAccountId) {
					if (auto userEntry = entry->settings.getUserEntry(username)) {
						if (userEntry->role == DefaultSharingType::Restricted || userEntry->role == DefaultSharingType::Viewer) {
							if (auto levelName = typeinfo_cast<CCTextInputNode*>(this->getChildByIDRecursive("level-name-input"))) {
								levelName->setTouchEnabled(false);
								levelName->setKeypadEnabled(false);
								if (levelName->m_textLabel) levelName->m_textLabel->setColor(ccc3(200, 200, 200));
								levelName->setKeyboardEnabled(false);
							}
							if (auto description = typeinfo_cast<CCTextInputNode*>(this->getChildByIDRecursive("description-input"))) {
								description->setTouchEnabled(false);
								description->setKeypadEnabled(false);
								if (description->m_textArea && description->m_textArea->m_label) {
									description->m_textArea->m_label->setCascadeColorEnabled(true);
									description->m_textArea->m_label->setColor(ccc3(200, 200, 200));
								}
								description->setKeyboardEnabled(false);
							}
						}
					}
				}

				std::vector<ui::Base*> children;

				children.push_back(new ui::MenuItemSpriteExtra {
					.store = reinterpret_cast<CCNode**>(&m_fields->m_joinButton),
					.callback = [=, this](auto) {
						auto const levelKey = entry->key;

						this->updateToGray();
						m_fields->m_levelKickedHandle = Dispatch<std::string_view>("alk.editor-collab/level-kicked").listen([=, this](std::string_view reason) {
							this->updateToNormal();
							geode::createQuickPopup("Editor Collab", fmt::format("You have been kicked from the level. Reason: {}", reason), "OK", nullptr, [](auto, auto) {}, true);
							return ListenerResult::Propagate;
						});

						m_fields->m_notification = Notification::create(
							"Joining: 0.00%",
							NotificationIcon::Loading,
							0.f
						);
						m_fields->m_notification->show();
						if (m_fields->m_textChanged) {
							if (BrowserManager::get()->isMyLevel(m_level)) {
								auto* entry = BrowserManager::get()->getLevelEntry(m_level);
								BrowserManager::get()->updateLevelEntry(m_level);

								m_fields->m_updateLevelListener.spawn(LevelManager::get()->updateLevelSettings(entry->key, entry->settings), [](auto res) {});
								this->joinLevel(levelKey);
							}
						}
						else {
							this->joinLevel(levelKey);
						}
						
					},
					.child = new ui::Sprite {
						.store = reinterpret_cast<CCNode**>(&m_fields->m_joinButtonSprite),
						.frameName = "EditServerButton.png"_spr,
					},
				});

				if (Mod::get()->getSettingValue<bool>("force-delete-button")) {
					children.push_back(new ui::MenuItemSpriteExtra {
						.store = reinterpret_cast<CCNode**>(&m_fields->m_deleteButton),
						.callback = [=, this](auto) {
							auto const levelKey = entry->key;

							m_fields->m_deleteLevelListener.spawn(LevelManager::get()->deleteLevel(levelKey), [=, this](auto res) {
								if (GEODE_UNWRAP_IF_ERR(err, res)) {
									//////// log::debug("delete level task error");
									geode::createQuickPopup("Editor Collab (Error)", err, "OK", nullptr, [](auto, auto) {}, true);
								}
								else {
									//////// log::debug("delete level task succeed");

									auto token = WebManager::get()->getLoginToken();
									Dispatch<std::string_view, uint32_t>(
										"delete-level"_spr).send(token, LevelManager::get()->getClientId());
								}
							});
						},
						.child = new ui::Sprite {
							.store = reinterpret_cast<CCNode**>(&m_fields->m_deleteButtonSprite),
							.frameName = "KickIcon.png"_spr,
						},
					});
				}

				

				auto gen = new ui::Container {
					.size = menu->getContentSize(),
					.child = new ui::Menu {
						.child = new ui::Row {
							.mainAxis = ui::MainAxisAlignment::Center,
							.children = children,
						},
					},
				};

				auto node = gen->get();
				node->setAnchorPoint(menu->getAnchorPoint());
				node->setPosition(menu->getPosition());
				this->addChild(node);

				if (WebManager::get()->isSocketConnected()) {
					m_fields->m_disconnectHandle = Dispatch<>("alk.editor-collab/socket-disconnected").listen([=, this]() {
						this->updateToNormal();
						return ListenerResult::Propagate;
					});
				}
			}
		}

		return true;
	}
};