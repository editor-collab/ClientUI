#include <hooks/ui/EditorPauseLayer.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/LevelUserList.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorPauseLayerUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorPauseLayer::init(editorLayer)) return false;

    if (!LevelManager::get()->getJoinedLevelKey().has_value()) {
        return true;
    }

    if (Mod::get()->getSavedValue<bool>("shown-editor-pause-tutorial") == false) {
        auto popup = geode::createQuickPopup(
            "Editor Collab", 
            "Here you can <ca>playtest (or in LDM)</c> or <cg>save the level</c> to your local. "
            "Also at the <cp>bottom</c>, you can find the <cb>User List</c> button.",
            "OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, false
        );
        popup->m_scene = this;
        popup->show();
        Mod::get()->setSavedValue("shown-editor-pause-tutorial", true);
    }
    
    this->setupGuidelinesMenu();
    this->setupInfoMenu();
    this->setupResumeMenu();

    return true;
}
void EditorPauseLayerUIHook::setupGuidelinesMenu() {
    if (auto entry = BrowserManager::get()->getOnlineEntry(m_editorLayer->m_level)) {
        auto accountId = GJAccountManager::get()->m_accountID;
        auto setting = &entry->settings;
        
        if (
            setting->hideUsers == false || 
            entry->hostAccountId == GJAccountManager::get()->m_accountID ||
            setting->getUserType(GJAccountManager::get()->m_username) == DefaultSharingType::Admin
        ) {
            auto gen = new ui::MenuItemSpriteExtra {
                .id = "user-list-button"_spr,
                .callback = [this](auto*){
                    auto entry = BrowserManager::get()->getLevelEntry(m_editorLayer->m_level);
                    if (entry) (void)LevelUserList::create(entry, m_editorLayer);
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
    if (auto entry = BrowserManager::get()->getOnlineEntry(m_editorLayer->m_level)) {
        auto sharingType = entry->settings.defaultSharing;
        std::string username = GJAccountManager::get()->m_username;
        auto accountId = GJAccountManager::get()->m_accountID;
        //////// log::debug("Editor Collab Status: accountId {}, username {}", accountId, username);

        for (auto& userEntry : entry->settings.users) {
            if (userEntry.name == username) {
                sharingType = userEntry.role;
                break;
            }
        }

        //////// log::debug("Editor Collab Status: hostAccountId {}, sharingType {}", entry->hostAccountId, static_cast<int>(sharingType));

        if (accountId == entry->hostAccountId) {
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

void EditorPauseLayerUIHook::setupResumeMenu() {
    auto saveAndPlay = static_cast<CCMenuItemSpriteExtra*>(this->querySelector("save-and-play-button"));
    auto saveAndExit = static_cast<CCMenuItemSpriteExtra*>(this->querySelector("save-and-exit-button"));
    auto save = static_cast<CCMenuItemSpriteExtra*>(this->querySelector("save-button"));
    auto exit = static_cast<CCMenuItemSpriteExtra*>(this->querySelector("exit-button"));

    auto playSprite = ButtonSprite::create("Play", 180, true, "goldFont.fnt", "GJ_button_01.png", 28.0f, 0.8f);
    auto playButton = CCMenuItemSpriteExtra::create(playSprite, this, menu_selector(EditorPauseLayerUIHook::onPlay));
    playButton->setID("play-button"_spr);
    auto saveToLocalSprite = ButtonSprite::create("Save To Local", 180, true, "goldFont.fnt", "GJ_button_01.png", 28.0f, 0.8f);
    auto saveToLocalButton = CCMenuItemSpriteExtra::create(saveToLocalSprite, this, menu_selector(EditorPauseLayerUIHook::onSaveToLocal));
    saveAndPlay->setTarget(this, menu_selector(EditorPauseLayerUIHook::onPlay));
    auto playInLDMSprite = ButtonSprite::create("Play in LDM", 180, true, "goldFont.fnt", "GJ_button_01.png", 28.0f, 0.8f);
    auto playInLDMButton = CCMenuItemSpriteExtra::create(playInLDMSprite, this, menu_selector(EditorPauseLayerUIHook::onPlayInLDM));
    saveAndExit->setTarget(this, menu_selector(EditorPauseLayerUIHook::onExitWithoutPrompt));
    auto exitWithoutPromptSprite = ButtonSprite::create("Exit", 180, true, "goldFont.fnt", "GJ_button_01.png", 28.0f, 0.8f);
    auto exitWithoutPromptButton = CCMenuItemSpriteExtra::create(exitWithoutPromptSprite, this, menu_selector(EditorPauseLayerUIHook::onExitWithoutPrompt));
    exitWithoutPromptButton->setID("exit-button"_spr);

    if (auto menu = static_cast<CCMenu*>(this->querySelector("resume-menu"))) {
        menu->insertAfter(playButton, saveAndPlay);
        menu->insertAfter(saveToLocalButton, saveAndExit);
        menu->insertAfter(playInLDMButton, save);
        menu->insertAfter(exitWithoutPromptButton, exit);
        saveAndPlay->setVisible(false);
        saveAndExit->setVisible(false);
        save->setVisible(false);
        exit->setVisible(false);
        menu->updateLayout();
    }

    // saveAndPlay->setNormalImage(playSprite);
	// saveAndPlay->setTarget(this, menu_selector(EditorPauseLayerUIHook::onPlay));

    // saveAndExit->setNormalImage(playInLDMSprite);
    // saveAndExit->setTarget(this, menu_selector(EditorPauseLayerUIHook::onPlayInLDM));

    // save->setNormalImage(saveToLocalSprite);
    // save->setTarget(this, menu_selector(EditorPauseLayerUIHook::onSaveToLocal));

    // exit->setTarget(this, menu_selector(EditorPauseLayerUIHook::onExitWithoutPrompt));
}

void EditorPauseLayerUIHook::onPlay(cocos2d::CCObject* sender) {
    if (m_fields->playLock) return;
    m_fields->playLock = true;
    
    m_editorLayer->m_level->m_lowDetailMode = false;
	m_editorLayer->m_level->m_lowDetailModeToggled = false;

    EditorPauseLayer::onSaveAndPlay(sender);

    m_fields->playLock = false;
}
void EditorPauseLayerUIHook::onPlayInLDM(cocos2d::CCObject* sender) {
    if (m_fields->playLock) return;
    m_fields->playLock = true;

    m_editorLayer->m_level->m_lowDetailMode = true;
	m_editorLayer->m_level->m_lowDetailModeToggled = true;

    EditorPauseLayer::onSaveAndPlay(sender);

    m_fields->playLock = false;
}
void EditorPauseLayerUIHook::onExitWithoutPrompt(cocos2d::CCObject* sender) {
    if (LevelManager::get()->getJoinedLevelKey().has_value()) {
        GameManager::get()->m_sceneEnum = 3;
    }
    
    EditorPauseLayer::onExitEditor(sender);
}

void EditorPauseLayerUIHook::saveLevel() {
    EditorPauseLayer::saveLevel();

    // if (LevelManager::get()->getJoinedLevelKey().has_value()) {
    //     BrowserManager::get()->saveLevel(m_editorLayer->m_level, false);
    // }
}

void EditorPauseLayerUIHook::onSaveToLocal(cocos2d::CCObject* sender) {
    if (m_fields->playLock) return;
    m_fields->playLock = true;

    EditorPauseLayer::saveLevel();
    BrowserManager::get()->saveLevel(m_editorLayer->m_level, true, false);

    m_fields->playLock = false;
}