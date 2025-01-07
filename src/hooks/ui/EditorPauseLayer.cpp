#include <hooks/ui/EditorPauseLayer.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/LevelUserList.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorPauseLayerUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorPauseLayer::init(editorLayer)) return false;

    if (!LevelManager::get()->getJoinedLevel().has_value()) {
        return true;
    }
    
    this->setupGuidelinesMenu();
    this->setupInfoMenu();

    return true;
}
void EditorPauseLayerUIHook::setupGuidelinesMenu() {
    auto gen = new ui::MenuItemSpriteExtra {
        .id = "user-list-button"_spr,
        .callback = [this](auto*){
            if (auto key = LevelManager::get()->getJoinedLevel()) {
                if (auto entry = BrowserManager::get()->getLevelEntry(*key)) {
                    auto userList = LevelUserList::create(*entry);
                }
            }
        },
        .child = new ui::Sprite {
            .frameName = "UserListButton.png"_spr,
        },
    };

    auto button = gen->get();
    button->setID("user-list-button"_spr);

    if (auto menu = static_cast<CCMenu*>(this->querySelector("guidelines-menu"))) {
        menu->addChild(button);
        menu->updateLayout();
    }
}
void EditorPauseLayerUIHook::setupInfoMenu() {
    if (auto objectLabel = static_cast<CCLabelBMFont*>(this->querySelector("object-count-label"))) {
        std::string str = objectLabel->getString();
        if (auto pos =str.find("Read-Only"); pos != std::string::npos) {
            str = str.replace(str.find("Read-Only"), 9, "Shared");
            objectLabel->setString(str.c_str());
        }
    }
    
    CCLabelBMFont* label = nullptr;
    if (auto key = LevelManager::get()->getJoinedLevel()) {
        if (auto entry = BrowserManager::get()->getLevelEntry(*key)) {
            auto sharingType = entry.value()->settings.defaultSharing;
            std::string username = GJAccountManager::get()->m_username;
            auto accountId = GJAccountManager::get()->m_accountID;

            for (auto& userEntry : entry.value()->settings.users) {
                if (userEntry.name == username) {
                    sharingType = userEntry.role;
                    break;
                }
            }

            if (accountId == entry.value()->hostAccountId) {
                label = CCLabelBMFont::create("Editor Collab Status: Host", "goldFont.fnt");
            }
            else {
                switch (sharingType) {
                    case DefaultSharingType::Restricted:
                        label = CCLabelBMFont::create("Editor Collab Status: Restricted", "goldFont.fnt");
                        break;
                    case DefaultSharingType::Viewer:
                        label = CCLabelBMFont::create("Editor Collab Status: Viewer", "goldFont.fnt");
                        break;
                    case DefaultSharingType::Editor:
                        label = CCLabelBMFont::create("Editor Collab Status: Editor", "goldFont.fnt");
                        break;
                    case DefaultSharingType::Admin:
                        label = CCLabelBMFont::create("Editor Collab Status: Admin", "goldFont.fnt");
                        break;
                }
            }
        }
    }

    if (!label) {
        return;
    }
    label->setID("editor-collab-status"_spr);

    label->setLayoutOptions(
        AxisLayoutOptions::create()
            ->setScaleLimits(0.1f, 0.5f)
            ->setBreakLine(true)
    );

    if (auto menu = static_cast<CCMenu*>(this->querySelector("info-menu"))) {
        menu->addChild(label);
        menu->updateLayout();
    }
}