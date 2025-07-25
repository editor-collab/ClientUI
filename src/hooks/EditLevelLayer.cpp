#include <hooks/EditLevelLayer.hpp>
#include <managers/LevelManager.hpp>
#include <managers/AccountManager.hpp>
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
		EventListener<Task<Result<>, WebProgress>> leaveLevelListener;		
	};

	void returnToLastScene(GJGameLevel* level) {
		if (LevelManager::get()->isInLevel()) {
			CameraValue camera;
			if (auto editorLayer = LevelEditorLayer::get()) {
				camera.x = editorLayer->m_objectLayer->getPositionX();
				camera.y = editorLayer->m_objectLayer->getPositionY();
				camera.zoom = editorLayer->m_objectLayer->getScale();
			}

			auto task = LevelManager::get()->leaveLevel(camera);
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
						"Cancel",
						"OK",
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
		bool m_textChanged = false;
		
		CCMenuItemSpriteExtra* m_joinButton = nullptr;
		CCSprite* m_joinButtonSprite = nullptr;
		CCMenuItemSpriteExtra* m_deleteButton = nullptr;
		CCSprite* m_deleteButtonSprite = nullptr;

		EventListener<DispatchFilter<>> m_disconnectListener = DispatchFilter<>("alk.editor-collab/socket-disconnected");
	};	

	$override
	void textChanged(CCTextInputNode* input) {
		EditLevelLayer::textChanged(input);
		if (BrowserManager::get()->isMyLevel(m_level)) {
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

				auto task = LevelManager::get()->updateLevelSettings(entry->key, entry->settings);
				task.listen([=](auto* result) {});
			}
		}
		EditLevelLayer::onBack(sender);
	}

    $override
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

		m_fields->m_textChanged = false;

		if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("level-edit-menu"))) {
			if (auto entry = BrowserManager::get()->getOnlineEntry(m_level)) {
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

						auto notification = Notification::create(
							"Joining: %0",
							NotificationIcon::Loading,
							0.f
						);
						notification->show();

						auto task = LevelManager::get()->joinLevel(levelKey);
						task.listen([=, this](auto* resultp) {
							if (GEODE_UNWRAP_EITHER(value, err, *resultp)) {
								log::debug("join level task succeed");
								notification->hide();

								auto token = AccountManager::get()->getLoginToken();
								DispatchEvent<std::string_view, uint32_t, std::string_view, std::vector<uint8_t> const*, std::optional<CameraValue>, GJGameLevel*>(
									"join-level"_spr, token, value.clientId, levelKey, &value.snapshot, value.camera, m_level
								).post();
							}
							else {
								log::debug("join level task error");
								createQuickPopup("Error", err, "Cancel", "OK", [](auto, auto) {});
							}
						}, [=, this](auto* progressP) {
							notification->setString(
								fmt::format("Joining: %{}", progressP->downloadProgress().value_or(0)).c_str()
							);
						}, [=, this]() {
							notification->hide();
						});
					},
					.child = new ui::Sprite {
						.store = reinterpret_cast<CCNode**>(&m_fields->m_joinButtonSprite),
						.frameName = "EditServerButton.png"_spr,
					},
				});

				children.push_back(new ui::MenuItemSpriteExtra {
					.store = reinterpret_cast<CCNode**>(&m_fields->m_deleteButton),
					.callback = [=, this](auto) {
						auto const levelKey = entry->key;

						auto task = LevelManager::get()->deleteLevel(levelKey);
						task.listen([=, this](auto* resultp) {
							if (GEODE_UNWRAP_IF_ERR(err, *resultp)) {
								log::debug("delete level task error");
								createQuickPopup("Error", err, "Cancel", "OK", [](auto, auto) {});
							}
							else {
								log::debug("delete level task succeed");

								auto token = AccountManager::get()->getLoginToken();
								DispatchEvent<std::string_view, uint32_t>(
									"delete-level"_spr, token, LevelManager::get()->getClientId()
								).post();
							}
						});
					},
					.child = new ui::Sprite {
						.store = reinterpret_cast<CCNode**>(&m_fields->m_deleteButtonSprite),
						.frameName = "KickIcon.png"_spr,
					},
				});

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
					m_fields->m_joinButton->setEnabled(false);
					m_fields->m_joinButtonSprite->setColor(ccColor3B { 100, 100, 100 });
					m_fields->m_deleteButton->setEnabled(false);
					m_fields->m_deleteButtonSprite->setColor(ccColor3B { 100, 100, 100 });
					m_fields->m_disconnectListener.bind([=, this]() {
						m_fields->m_joinButton->setEnabled(true);
						m_fields->m_joinButtonSprite->setColor(ccColor3B { 255, 255, 255 });
						m_fields->m_deleteButton->setEnabled(true);
						m_fields->m_deleteButtonSprite->setColor(ccColor3B { 255, 255, 255 });

						return ListenerResult::Propagate;
					});
				}
			}
		}

		return true;
	}
};