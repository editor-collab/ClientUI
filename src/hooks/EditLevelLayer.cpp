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
		
		CCMenuItemSpriteExtra* m_joinButton = nullptr;
		CCSprite* m_joinButtonSprite = nullptr;

		EventListener<DispatchFilter<>> m_disconnectListener = DispatchFilter<>("alk.editorcollab/socket-disconnected");
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
			auto shadowLevel = BrowserManager::get()->getShadowLevel(m_level);
			if (BrowserManager::get()->isShadowLevel(shadowLevel)) {
				BrowserManager::get()->setLevelValues(shadowLevel, *entry);
			}
		}
	}

	$override
	void onBack(CCObject* sender) {
		if (m_fields->m_textChanged) {
			auto const* entry = BrowserManager::get()->getLevelEntry(m_level);
			BrowserManager::get()->saveLevelEntry(*entry);

			if (entry->isShared()) {
				auto task = LevelManager::get()->updateLevelSettings(
					BrowserManager::get()->getLevelKey(m_level).value(),
					entry->settings
				);
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
			auto entry = BrowserManager::get()->getLevelEntry(m_level);
			if (entry && entry->isShared()) {
				for (auto child : CCArrayExt<CCNode*>(menu->getChildren())) {
					child->setVisible(false);
				}

				std::vector<ui::Base*> children;

				children.push_back(new ui::MenuItemSpriteExtra {
					.store = reinterpret_cast<CCNode**>(&m_fields->m_joinButton),
					.callback = [=, this](auto) {
						auto const levelKey = entry->key;

						auto shadowLevel = BrowserManager::get()->getShadowLevel(m_level);
						log::debug("shadow of {} is {}", m_level, shadowLevel);

						auto task = LevelManager::get()->joinLevel(levelKey);
						task.listen([=, this](auto* resultp) {
							if (GEODE_UNWRAP_EITHER(value, err, *resultp)) {
								log::debug("join level task succeed");

								auto token = AccountManager::get()->getLoginToken();
								DispatchEvent<std::string_view, uint32_t, std::string_view, std::vector<uint8_t> const*, std::optional<CameraValue>, GJGameLevel*>(
									"join-level"_spr, token, value.clientId, levelKey, &value.snapshot, value.camera, shadowLevel
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
					m_fields->m_disconnectListener.bind([=, this]() {
						m_fields->m_joinButton->setEnabled(true);
						m_fields->m_joinButtonSprite->setColor(ccColor3B { 255, 255, 255 });

						return ListenerResult::Propagate;
					});
				}
			}
		}

		return true;
	}
};