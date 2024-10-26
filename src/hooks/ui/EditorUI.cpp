#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/ShareSettings.hpp>
#include <alk.lavender/include/lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorUIUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;
    
    auto gen = new ui::MenuItemSpriteExtra {
        .id = "share-button"_spr,
        .callback = [this](auto*){
            if (auto key = LevelManager::get()->getJoinedLevel()) {
                if (auto entry = BrowserManager::get()->getLevelEntry(*key)) {
                    auto shareSettings = ShareSettings::create(*entry);
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

