#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/ShareSettings.hpp>
#include <ui/BuyPopup.hpp>
#include <ui/LevelUserList.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorUIUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;

    auto realLevel = editorLayer->m_level;
    if (BrowserManager::get()->isShadowLevel(editorLayer->m_level)) {
        realLevel = BrowserManager::get()->getReflectedLevel(editorLayer->m_level);
        log::debug("On a shadow level, using real level {}", realLevel);
    }
    else {
        log::debug("On a real level {}", realLevel);
    }

    auto gen = new ui::MenuItemSpriteExtra {
        .id = "share-button"_spr,
        .callback = [=, this](auto*){
            auto const sharedLevels = BrowserManager::get()->getShadowMyLevels();
            auto const hostableCount = FetchManager::get()->getHostableCount();
            auto const key = LevelManager::get()->getJoinedLevelKey();
            LevelEntry* entry = nullptr;
            if (key) {
                entry = BrowserManager::get()->getLevelEntry(*key);
            }
            else {
                entry = BrowserManager::get()->getLevelEntry(realLevel);
            }
            if (entry == nullptr) {
                BrowserManager::get()->addLevelEntry(realLevel, LevelEntry {});
                entry = BrowserManager::get()->getLevelEntry(realLevel);
            }


            // TODO: cleanup
            if (key && BrowserManager::get()->isMyLevel(*key) || sharedLevels->count() < hostableCount) {
                (void)ShareSettings::create(entry, m_editorLayer);
            }
            else {
                std::string desc;
                if (hostableCount > 0) {
                    std::vector<std::string> levelNames;
                    for (auto const& level : CCArrayExt<GJGameLevel*>(sharedLevels)) {
                        levelNames.push_back(level->m_levelName);
                    }
                    desc = fmt::format(
                        "You have filled up <cr>{}/{}</c> sharing slots with <cy>{}</c>. "
                        "You can either stop sharing one or <cg>buy more slots</c>.",
                        sharedLevels->count(), hostableCount, fmt::join(levelNames, ", ")
                    );
                    createQuickPopup("Editor Collab", desc, "Cancel", "Buy", [this](auto*, bool isBtn2) {
                        if (isBtn2)  {
                            (void)BuyPopup::create();
                        }
                    }, true);   
                }
                else {
                    (void)BuyPopup::create();
                }
            }
        },
        .child = new ui::Sprite {
            .frameName = "ShareButton.png"_spr,
        },
    };

    auto button = gen->get();
    m_fields->m_shareButton = static_cast<CCMenuItemSpriteExtra*>(button);

    if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("settings-menu"))) {
        menu->addChild(button);
        menu->updateLayout();
    }

    return true;
}

void EditorUIUIHook::showUI(bool show) {
    EditorUI::showUI(show);    
    if (m_fields->m_shareButton) {
        m_fields->m_shareButton->setVisible(show);
    }
}

