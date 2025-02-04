#include <ui/ShareSettings.hpp>
#include <ui/LimitsSettings.hpp>
#include <managers/LevelManager.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

static constexpr auto POPUP_SIZE = CCSize{400.f, 290.f};

ShareSettings* ShareSettings::create(LevelEntry* entry) {
    auto ret = new (std::nothrow) ShareSettings();
    if (ret && ret->init(entry)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ShareSettings::init(LevelEntry* entry) {
    if (!CCNode::init()) return false;

    m_entry = entry;
    m_setting = &entry->settings;

    auto shareRow = new ui::Menu {
        .child = new ui::Row {
            .id = "share-row",
            .children = {
                new ui::Expanded {
                    .child = new ui::Row {
                        .children = {
                            new ui::Flexible {
                                .child = new ui::FittedBox {
                                    .child = new ui::TextArea {
                                        .store = reinterpret_cast<CCNode**>(&m_shareDescription),
                                        .id = "share-description",
                                        .alignment = ui::TextAlignment::Left,
                                        .scale = 0.8f,
                                        .text = "Share your level with others!",
                                    },
                                    .fit = ui::BoxFit::ScaleDown,
                                },
                            },
                        },
                    },
                },
                new ui::Container {.width = 10},
                new ui::MenuItemSpriteExtra {
                    .callback = [this](auto*) {

                    },
                    .child = new ui::Scale9Sprite {
                        .fileName = "GJ_button_01.png",
                        .child = new ui::Container {
                            .padding = ui::EdgeInsets::All{7.f},
                            .child = new ui::TextArea {
                                .text = "Share",
                                .font = "bigFont.fnt",
                                .scale = 0.5f,
                            },
                        },
                    },
                },
            },
        },
    };

    auto shareWith = new ui::Menu {
        .child = new ui::Row {
            .id = "share-with-row",
            .children = {
                new ui::Expanded {
                    .child = new ui::TextInput {
                        .store = reinterpret_cast<CCNode**>(&m_shareWithInput),
                        .id = "share-with-input",
                        .placeholder = "Share with...",
                    },
                },
                new ui::Container {.width = 10},
                new ui::MenuItemSpriteExtra {
                    // TODO: add ButtonSprite to Lavender
                    .callback = [this](auto*) {
                        this->addSharedUser(nullptr);
                    },
                    .child = new ui::Scale9Sprite {
                        .fileName = "GJ_button_01.png",
                        .child = new ui::Container {
                            .padding = ui::EdgeInsets::All{7.f},
                            .child = new ui::TextArea {
                                .text = "Add",
                                .font = "bigFont.fnt",
                                .scale = 0.5f,
                            },
                        },
                    },
                },
            },
        },
    };

    auto userList = new ui::Scale9Sprite {
        .fileName = "square02b_001.png",
        .scale = .5f,
        .color = ccc4(0x00, 0x00, 0x00, 0x5a),
        .size = CCSize{FLT_MAX, FLT_MAX},
        .child = new ui::Container {
            .child = new ui::ScrollLayer {
                .store = reinterpret_cast<CCNode**>(&m_peopleScrollLayer),
                .id = "people-scroll-layer",
                .count = m_setting->users.size(),
                .builder = [=, this](auto idx) -> ui::Base* {
                    return this->generatePersonEntry(m_setting->users[idx].name, m_setting->users[idx].role);
                },
            },
        },
    };

    auto generalAccessType = new ui::Row {
        .store = &m_generalAccessTypeRow,
        .id = "general-access-type-row",
        .children = {
            new ui::MenuItemSpriteExtra {
                .id = "general-access-type-change-button-left",
                .callback = [this](auto*){
                    switch (m_setting->defaultSharing) {
                        // case DefaultSharingType::Viewer:
                        //     m_setting->defaultSharing = DefaultSharingType::Editor;
                        //     break;
                        // case DefaultSharingType::Editor:
                        //     m_setting->defaultSharing = DefaultSharingType::Viewer;
                        //     break;
                        default:
                            m_setting->defaultSharing = DefaultSharingType::Viewer;
                            break;
                    }
                    this->updateValues();
                },
                .child = new ui::Sprite {
                    .frameName = "GJ_arrow_01_001.png",
                    .scale = 0.45f
                },
            },
            new ui::Container {.width = 4},
            new ui::TextArea {
                .store = reinterpret_cast<CCNode**>(&m_generalAccessTypeText),
                .id = "general-access-type-text",
                .text = "",
                .font = "bigFont.fnt",
                .scale = 0.45f,
            },
            new ui::Container {.width = 4},
            new ui::MenuItemSpriteExtra {
                .id = "general-access-type-change-button-right",
                .callback = [this](auto*){
                    switch (m_setting->defaultSharing) {
                        // case DefaultSharingType::Viewer:
                        //     m_setting->defaultSharing = DefaultSharingType::Editor;
                        //     break;
                        // case DefaultSharingType::Editor:
                        //     m_setting->defaultSharing = DefaultSharingType::Viewer;
                        //     break;
                        default:
                            m_setting->defaultSharing = DefaultSharingType::Viewer;
                            break;
                    }
                    this->updateValues();
                },
                .child = new ui::Sprite {
                    .frameName = "GJ_arrow_01_001.png",
                    .scaleX = -0.45f,
                    .scaleY = 0.45f,
                },
            },
        },
    };

    auto generalAccessColumn = new ui::Column {
        .id = "general-access-change-column",
        .crossAxis = ui::CrossAxisAlignment::Start,
        .children = {
            new ui::Row {
                .id = "general-access-change-row",
                .children = {
                    new ui::MenuItemSpriteExtra {
                        .id = "general-access-change-button-left",
                        .callback = [this](auto*) {
                            this->changeGeneralAccess(nullptr);
                        },
                        .child = new ui::Sprite {
                            .frameName = "GJ_arrow_01_001.png",
                            .scale = 0.35f
                        },
                    },
                    new ui::Container {.width = 4},
                    new ui::TextArea {
                        .store = reinterpret_cast<CCNode**>(&m_generalAccessText),
                        .id = "general-access-text",
                        .text = "",
                        .font = "bigFont.fnt",
                        .scale = 0.35f,
                    },
                    new ui::Container {.width = 4},
                    new ui::MenuItemSpriteExtra {
                        .id = "general-access-change-button-right",
                        .callback = [this](auto*) {
                            this->changeGeneralAccess(nullptr);
                        },
                        .child = new ui::Sprite {
                            .frameName = "GJ_arrow_01_001.png",
                            .scaleX = -0.35f,
                            .scaleY = 0.35f,
                        },
                    },
                },
            },
            new ui::Container {.height = 4},
            new ui::TextArea {
                .store = reinterpret_cast<CCNode**>(&m_generalAccessDescription),
                .id = "general-access-description",
                .text = "",
                .font = "chatFont.fnt",
                .scale = 0.55f,
            },
        },
    };

    auto gen = new ui::Popup {
        .size = POPUP_SIZE,
        .id = "share-settings-popup",
        .bgId = "share-settings-bg",
        .closeId = "share-settings-close",
        .child = new ui::Center {
            .store = reinterpret_cast<CCNode**>(&m_center),
            .id = "share-settings-center",
            .child = new ui::Container {
                .id = "share-settings-container",
                .padding = ui::EdgeInsets::Symmetric{.horizontal = 20.f, .vertical = 10.f},
                .child = new ui::Column {
                    .id = "share-settings-column",
                    .children = {
                        new ui::Row {
                            .id = "share-settings-row",
                            .mainAxis = ui::MainAxisAlignment::Center,
                            .children = {
                                new ui::TextArea {
                                    .id = "share-settings-title",
                                    .text = fmt::format("Share \"{}\"", m_setting->title),
                                    .font = "bigFont.fnt",
                                    .scale = 0.7f,
                                },
                            },
                        },
                        // new ui::Container {.height = 10},
                        // shareRow,
                        new ui::Container {.height = 10},
                        shareWith,
                        new ui::Container {.height = 10},
                        new ui::TextArea {
                            .id = "people-with-access-title",
                            .text = "People with access",
                            .font = "bigFont.fnt",
                            .scale = 0.55f,
                        },
                        new ui::Container {.height = 5},
                        new ui::Flexible {
                            .child = userList,
                        },
                        new ui::Container {.height = 10},
                        new ui::TextArea {
                            .id = "general-access-title",
                            .text = "General access",
                            .font = "bigFont.fnt",
                            .scale = 0.55f,
                        },
                        new ui::Container {.height = 5},
                        new ui::Menu {
                            .child = new ui::Scale9Sprite {
                                .fileName = "square02b_001.png",
                                .scale = .5f,
                                .color = ccc4(0x00, 0x00, 0x00, 0x5a),
                                .child = new ui::Container {
                                    .padding = ui::EdgeInsets::All{5.f},
                                    .child = new ui::Row {
                                        .id = "general-access-row",
                                        .children = {
                                            new ui::Sprite {
                                                .id = "general-access-icon",
                                                .frameName = "ViewIcon.png"_spr,
                                                .scale = 1.f,
                                            },
                                            new ui::Container {.width = 8},
                                            generalAccessColumn,
                                            new ui::Expanded {},
                                            generalAccessType,
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            },
        },
    };

    m_popup = gen->get();
    m_popup->addChild(this);

    this->updateValues();

    return true;
}

void ShareSettings::updateValues() {
    if (m_generalAccessText) {
        if (m_setting->discoverable) {
            m_generalAccessText->setText("Anyone from Discover");
        } else {
            m_generalAccessText->setText("Restricted");
        }
    }

    if (m_generalAccessTypeRow) {
        if (m_setting->discoverable) {
            m_generalAccessTypeRow->setVisible(true);
        } else {
            m_generalAccessTypeRow->setVisible(false);
        }
    }

    if (m_generalAccessTypeText) {
        m_generalAccessTypeText->setText(this->getSharingTypeString(m_setting->defaultSharing));
    }

    if (m_generalAccessDescription) {
        if (m_setting->discoverable) {
            m_generalAccessDescription->setText("Anyone can join from the Discover tab");
        } else {
            m_generalAccessDescription->setText("Only people with access can join");
        }
    }

    cocos2d::CCPoint scrollPosition;
    if (m_peopleScrollLayer) {
        scrollPosition = m_peopleScrollLayer->m_contentLayer->getPosition();
    }

    m_center->updateLayout();

    if (m_peopleScrollLayer && !m_resetScroll) {
        m_peopleScrollLayer->m_contentLayer->setPosition(scrollPosition);
    }
    else {
        m_resetScroll = false;
    }

    auto task = LevelManager::get()->updateLevelSettings(
        m_entry->key,
        LevelSetting(*m_setting)
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

std::string ShareSettings::getSharingTypeString(DefaultSharingType type) {
    switch (type) {
        case DefaultSharingType::Restricted:
            return "Restricted";
        case DefaultSharingType::Viewer:
            return "Viewer";
        case DefaultSharingType::Editor:
            return "Editor";
        case DefaultSharingType::Admin:
            return "Admin";
    }
}

ui::Base* ShareSettings::generatePersonEntry(std::string name, DefaultSharingType type) {
    return new ui::Container {
        .padding = ui::EdgeInsets::All{4.f},
        .id = name + "-entry",
        .child = new ui::Menu {
            .id = name + "-menu",
            .child = new ui::Row {
                .id = name + "-row",
                .children = {
                    new ui::TextArea {
                        .id = name + "-access-text",
                        .text = std::string(name),
                        .font = "bigFont.fnt",
                        .scale = 0.50f,
                    },
                    new ui::Expanded {},
                    new ui::MenuItemSpriteExtra {
                        .id = name + "-change-button-left",
                        .callback = [this, name](auto*){
                            auto type = m_setting->getUserType(name);
                            switch (type) {
                                case DefaultSharingType::Viewer:
                                    m_setting->setUser(name, DefaultSharingType::Admin);
                                    break;
                                case DefaultSharingType::Editor:
                                    m_setting->setUser(name, DefaultSharingType::Viewer);
                                    break;
                                case DefaultSharingType::Admin:
                                    m_setting->setUser(name, DefaultSharingType::Editor);
                                    break;
                                default:
                                    m_setting->setUser(name, DefaultSharingType::Viewer);
                                    break;
                            }
                            if (auto label = static_cast<geode::SimpleTextArea*>(m_peopleScrollLayer->getChildByIDRecursive(name + "-sharing-text"))) {
                                label->setText(this->getSharingTypeString(m_setting->getUserType(name)));
                            }
                            this->updateValues();
                        },
                        .child = new ui::Sprite {
                            .frameName = "GJ_arrow_01_001.png",
                            .scale = 0.45f
                        },
                    },
                    new ui::Container {.width = 4},
                    new ui::TextArea {
                        .id = name + "-sharing-text",
                        .text = this->getSharingTypeString(type),
                        .font = "bigFont.fnt",
                        .scale = 0.45f,
                    },
                    new ui::Container {.width = 4},
                    new ui::MenuItemSpriteExtra {
                        .id = name + "-change-button-right",
                        .callback = [this, name](auto*){
                            auto type = m_setting->getUserType(name);
                            switch (type) {
                                case DefaultSharingType::Viewer:
                                    m_setting->setUser(name, DefaultSharingType::Editor);
                                    break;
                                case DefaultSharingType::Editor:
                                    m_setting->setUser(name, DefaultSharingType::Admin);
                                    break;
                                case DefaultSharingType::Admin:
                                    m_setting->setUser(name, DefaultSharingType::Viewer);
                                    break;
                                default:
                                    m_setting->setUser(name, DefaultSharingType::Viewer);
                                    break;
                            }
                            if (auto label = static_cast<geode::SimpleTextArea*>(m_peopleScrollLayer->getChildByIDRecursive(name + "-sharing-text"))) {
                                label->setText(this->getSharingTypeString(m_setting->getUserType(name)));
                            }
                            this->updateValues();
                        },
                        .child = new ui::Sprite {
                            .frameName = "GJ_arrow_01_001.png",
                            .scaleX = -0.45f,
                            .scaleY = 0.45f,
                        },
                    },
                    // new ui::MenuItemSpriteExtra {
                    //     .id = name + "-limits-button",
                    //     .callback = [this, name](auto*){
                    //         if (auto entry = m_setting->getUserEntry(name)) {
                    //             LimitsSettings::create(m_entry->key, m_setting, entry);
                    //         }
                    //     },
                    //     .child = new ui::Sprite {
                    //         .frameName = "GJ_deleteIcon_001.png",
                    //         .scale = .6f,
                    //     },
                    // },
                    new ui::Container {.width = 8},
                    new ui::MenuItemSpriteExtra {
                        .id = name + "-delete-button",
                        .callback = [this, name](auto*){
                            m_setting->removeUser(name);
                            if (auto child = m_peopleScrollLayer->getChildByIDRecursive(name + "-entry")) {
                                child->removeFromParent();
                            }   
                            this->updateValues();
                        },
                        .child = new ui::Sprite {
                            .frameName = "GJ_deleteIcon_001.png",
                            .scale = .6f,
                        },
                    },
                },
            },
        },
    };
}

void ShareSettings::addSharedUser(cocos2d::CCObject* sender) {
    auto name = m_shareWithInput->getString();
    if (name.empty()) return;
    if (m_setting->hasUser(name)) return;

    m_setting->setUser(name, DefaultSharingType::Viewer);
    if (m_peopleScrollLayer) {
        m_peopleScrollLayer->m_contentLayer->addChild(this->generatePersonEntry(name, DefaultSharingType::Viewer)->get());
    }
    m_shareWithInput->setString("");
    m_resetScroll = true;
    this->updateValues();
    if (m_peopleScrollLayer) {
        m_peopleScrollLayer->m_contentLayer->setPositionY(0);
    }
}

void ShareSettings::changeGeneralAccess(cocos2d::CCObject* sender) {
    m_setting->discoverable = !m_setting->discoverable;
    if (m_setting->discoverable) {
        m_setting->defaultSharing = DefaultSharingType::Viewer;
    }
    else {
        m_setting->defaultSharing = DefaultSharingType::Restricted;
    }
    this->updateValues();
}