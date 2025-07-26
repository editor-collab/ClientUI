#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/ShareSettings.hpp>
#include <ui/BuyPopup.hpp>
#include <ui/LevelUserList.hpp>
#include <lavender/Lavender.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorUIUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;

    auto gen = new ui::MenuItemSpriteExtra {
        .id = "share-button"_spr,
        .callback = [=, this](auto*){
            auto const sharedLevels = BrowserManager::get()->getMyLevels();
            auto const hostableCount = FetchManager::get()->getHostableCount();

            if (!BrowserManager::get()->hasLevelEntry(editorLayer->m_level)) {
                BrowserManager::get()->addLevelEntry(editorLayer->m_level, LevelEntry {});
                //////// log::debug("Added new level entry for level {}", editorLayer->m_level);
            }
            LevelEntry* entry = BrowserManager::get()->getLevelEntry(editorLayer->m_level);
            entry->hostAccountId = GJAccountManager::get()->m_accountID;

            // TODO: cleanup
            if (BrowserManager::get()->isMyLevel(editorLayer->m_level) || sharedLevels->count() < hostableCount) {
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

