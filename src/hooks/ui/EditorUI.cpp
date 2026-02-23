#include <hooks/ui/EditorUI.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <ui/ShareSettings.hpp>
#include <ui/BuyPopup.hpp>
#include <ui/LevelUserList.hpp>
#include <lavender/Lavender.hpp>
#include <managers/WebManager.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

bool EditorUIUIHook::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;

    if (!WebManager::get()->isLoggedIn()) {
        m_fields->m_shareButton = nullptr;
        return true;
    }

    if (Mod::get()->getSavedValue<bool>("shown-share-introduction-popup") == false) {
        auto popup = geode::createQuickPopup(
            "Editor Collab", 
            "To <cg>share</c> your levels, press the <cj>Share Button</c> at the <cp>top right corner</c> of the editor to open the <cb>Share Popup</c>.",
            "OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, false
        );
        popup->m_scene = this;
        popup->show();
        Mod::get()->setSavedValue("shown-share-introduction-popup", true);
    }

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
            if (entry->settings.getUserType(GJAccountManager::get()->m_username) == DefaultSharingType::Admin
                || BrowserManager::get()->isMyLevel(editorLayer->m_level)
                || sharedLevels->count() < hostableCount) {
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
                    geode::createQuickPopup("Editor Collab", desc, "Cancel", "Buy", [this](auto*, bool isBtn2) {
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

    if (auto gridSizeControls = static_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("hjfod.betteredit/grid-size-controls"))) {
        gridSizeControls->setPosition(gridSizeControls->getPosition() + ccp(-40.f, -20.f));
    }

    auto button = gen->get();
    m_fields->m_shareButton = static_cast<CCMenuItemSpriteExtra*>(button);
    m_uiItems->addObject(button);    

    if (auto menu = static_cast<CCMenu*>(this->getChildByIDRecursive("settings-menu"))) {
        menu->addChild(button);
        menu->updateLayout();
    }

    m_fields->uiVisibilityHandle = Dispatch<bool>("alk.editor-collab/set-ui-visibility").listen([this](bool visible) {
        this->setVisibility(visible);
        return ListenerResult::Propagate;
    });

    return true;
}

void EditorUIUIHook::showUI(bool show) {
    show = show && m_fields->visibility;
    EditorUI::showUI(show);    
    if (m_fields->m_shareButton) {
        m_fields->m_shareButton->setVisible(show);
    }

    // fixing robtop's bugs? in my editor collab? never...
    m_goToBaseBtn->setVisible(show);
    m_linkBtn->setVisible(show);
    m_unlinkBtn->setVisible(show);
    m_enableLinkBtn->setVisible(show);
}

void EditorUIUIHook::setVisibility(bool show) {
    m_fields->visibility = show;

    CCMenuItemSpriteExtra* hideUIToggle = static_cast<CCMenuItemSpriteExtra*>(this->querySelector("hjfod.betteredit/hide-ui-toggle"));

	if (show) {
        if (m_fields->savedButtonBar) {
            this->toggleMode(m_fields->savedButtonBar);
        }
        else {
            this->toggleMode(m_createButtonBar);
        }
		this->showUI(true);
		this->m_playtestBtn->setVisible(true);
        if (hideUIToggle) hideUIToggle->setVisible(true);
	}
	else {
        m_fields->savedButtonBar = m_customTabBar;
        this->toggleMode(m_editButtonBar);
		this->showUI(false);
		this->m_playtestBtn->setVisible(false);
        if (hideUIToggle) hideUIToggle->setVisible(false);
	}
    this->updateButtons();
}