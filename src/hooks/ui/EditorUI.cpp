#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/ShareSettings.hpp>
#include <ui/LevelUserList.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorUIUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;

    auto gen = new ui::MenuItemSpriteExtra {
        .id = "share-button"_spr,
        .callback = [this](auto*){
            auto const& lastMyLevels = FetchManager::get()->getLastMyLevels();
            auto const hostableCount = FetchManager::get()->getHostableCount();
            auto const key = LevelManager::get()->getJoinedLevel();

            auto const inSharedLevel = std::count_if(lastMyLevels.begin(), lastMyLevels.end(), [&](auto const& level) {
                return key.has_value() && level.key == *key;
            }) > 0;

            if (lastMyLevels.size() < hostableCount || inSharedLevel && key.has_value()) {
                if (auto entry = BrowserManager::get()->getLevelEntry(*key)) {
                    (void)ShareSettings::create(*entry);
                }
            } else {
                std::string desc;
                if (hostableCount > 0) {
                    std::vector<std::string> levelNames;
                    for (auto const& level : lastMyLevels) {
                        levelNames.push_back(level.settings.title);
                    }
                    desc = fmt::format(
                        "You have filled up <cr>{}/{}</c> sharing slots with <cy>{}</c>. "
                        "You can either stop sharing one or <cg>buy more slots</c>.",
                        lastMyLevels.size(), hostableCount, fmt::join(levelNames, ", ")
                    );
                }
                else {
                    desc = "You dont have any available sharing slots. "
                           "You can <cg>buy sharing slots</c> to share levels.";
                }
                createQuickPopup("Editor Collab", desc, "Buy", "Cancel", [this](auto*, bool) {
                    AppDelegate::get()->openURL("https://buy.stripe.com/aEUbLb38R2Cw91K9AA");
                }, true);   
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

