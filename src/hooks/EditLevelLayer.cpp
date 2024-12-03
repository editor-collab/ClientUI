#include <hooks/EditLevelLayer.hpp>
#include <managers/LevelManager.hpp>
#include <managers/AccountManager.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/WebManager.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <alk.lavender/include/lavender/Lavender.hpp>

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
		bool m_textChanged = false;
		bool m_levelDeleted = false;
		
		CCMenuItemSpriteExtra* m_joinButton = nullptr;
		CCSprite* m_joinButtonSprite = nullptr;

		struct Destructor {
			~Destructor() {
				WebManager::get()->clearSocketCallbacks();
			}
		} m_destructor;
	};

	$override
	void textChanged(CCTextInputNode* input) {
		EditLevelLayer::textChanged(input);
		log::debug("levle {}", m_level);
		if (BrowserManager::get()->isMyLevel(m_level)) {
			log::debug("my levle {}", m_level);
			m_fields->m_textChanged = true;
			auto* entry = BrowserManager::get()->getLevelEntry(m_level).value();
			if (input->getTag() == 1) {
				entry->settings.title = input->getString();
			}
			else if (input->getTag() == 2) {
				entry->settings.description = input->getString();
			}
		}
	}

	$override
	void onBack(CCObject* sender) {
		if (!m_fields->m_levelDeleted && m_fields->m_textChanged) {
			auto const* entry = BrowserManager::get()->getLevelEntry(m_level).value();
			auto task = LevelManager::get()->updateLevelSettings(
				BrowserManager::get()->getLevelKey(m_level).value(),
				LevelSetting(entry->settings)
			);
			task.listen([=](auto* result) {
				// if (result->isOk()) {
				// 	createQuickPopup("Success", "Level settings updated", "OK", "Cancel", [](auto, auto) {});
				// }
				// else {
				// 	createQuickPopup("Error", result->unwrapErr(), "OK", "Cancel", [](auto, auto) {});
				// }
			});
		}
		EditLevelLayer::onBack(sender);
	}

    $override
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

		m_fields->m_textChanged = false;

		if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("level-edit-menu"))) {
			auto entry = BrowserManager::get()->getLevelEntry(m_level);
			if (entry.has_value()) {
				for (auto child : CCArrayExt<CCNode*>(menu->getChildren())) {
					child->setVisible(false);
				}

				std::vector<ui::Base*> children;
				if (BrowserManager::get()->isMyLevel(m_level)) {
					children.push_back(new ui::MenuItemSpriteExtra {
						.callback = [=, this](auto) {
							auto const levelKey = entry.value()->key;

							auto task = LevelManager::get()->deleteLevel(levelKey);
							task.listen([=, this](Result<>* resultp) {
								if (resultp->isErr()) {
									createQuickPopup("Error", resultp->unwrapErr(), "OK", "Cancel", [](auto, auto) {});
									return;
								}
								m_fields->m_levelDeleted = true;
								auto token = AccountManager::get()->getLoginToken();
								DispatchEvent<std::string_view, uint32_t>(
									"delete-level"_spr, token, LevelManager::get()->getClientId()
								).post();
								createQuickPopup("Success", "Level deleted", "OK", "Cancel", [](auto, auto) {});
							});
						},
						.child = new ui::Node {
							.node = CircleButtonSprite::create(CCLabelBMFont::create("Delete", "bigFont.fnt"), CircleBaseColor::DarkPurple),
						},
					});
					children.push_back(new ui::Container {
						.width = 10,
					});
				}

				children.push_back(new ui::MenuItemSpriteExtra {
					.store = reinterpret_cast<CCNode**>(&m_fields->m_joinButton),
					.callback = [=, this](auto) {
						auto const levelKey = entry.value()->key;

						auto task = LevelManager::get()->joinLevel(levelKey);
						task.listen([=, this](auto* resultp) {
							if (GEODE_UNWRAP_EITHER(value, err, *resultp)) {
								log::debug("join level task succeed");

								auto token = AccountManager::get()->getLoginToken();
								DispatchEvent<std::string_view, uint32_t, std::string_view, std::vector<uint8_t> const*, std::optional<CameraValue>>(
									"join-level"_spr, token, value.clientId, levelKey, &value.snapshot, value.camera
								).post();
							}
							else {
								log::debug("join level task error");
								createQuickPopup("Error", err, "OK", "Cancel", [](auto, auto) {});
							}
						});
					},
					.child = new ui::Sprite {
						.store = reinterpret_cast<CCNode**>(&m_fields->m_joinButtonSprite),
						.frameName = "EditServerButton.png"_spr,
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
					WebManager::get()->runOnSocketDisconnected([this] {
						m_fields->m_joinButton->setEnabled(true);
						m_fields->m_joinButtonSprite->setColor(ccColor3B { 255, 255, 255 });
					});
				}
			}
		}

        if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("folder-menu"))) {
			auto alternate = Mod::get()->getSettingValue<bool>("alt-button");
			auto menuSprite = CCSprite::createWithSpriteFrameName(
				alternate ? "AlternateMenuButton.png"_spr : "MenuButton.png"_spr
			);
			menuSprite->setScale(0.9f);

			auto gen = new ui::MenuItemSpriteExtra {
				.callback = [=, this](auto) {
					auto task = LevelManager::get()->createLevel(0, EditorIDs::getID(level), LevelSetting::fromLevel(level));
					task.listen([=, this](auto* resultp) {
						if (GEODE_UNWRAP_EITHER(value, err, *resultp)) {
							auto token = AccountManager::get()->getLoginToken();
							DispatchEvent<std::string_view, uint32_t, GJGameLevel*, std::string_view>(
								"create-level"_spr, token, value.clientId, level, value.levelKey
							).post();
						}
						else {
							createQuickPopup("Error", err, "OK", "Cancel", [](auto, auto) {});
						}
					});
				},
				.child = new ui::Node {
					.node = CircleButtonSprite::create(CCLabelBMFont::create("Create", "bigFont.fnt"), CircleBaseColor::Cyan),
				},
			};

			menu->addChild(gen->get());
			menu->updateLayout();
		}
		return true;
	}
};