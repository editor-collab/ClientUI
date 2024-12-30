#include <ui/ShareSettings.hpp>
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

    auto shareWith = new ui::Menu {
        .child = new ui::Row {
            .id = "share-with-row"_spr,
            .children = {
                new ui::Expanded {
                    .child = new ui::TextInput {
                        .store = reinterpret_cast<CCNode**>(&m_shareWithInput),
                        .id = "share-with-input"_spr,
                        .placeholder = "Share with...",
                    },
                },
                new ui::Container {.width = 10},
                new ui::MenuItemSpriteExtra {
                    // TODO: add ButtonSprite to Lavender
                    .callback = [this](auto*) {
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
                    },
                    .child = new ui::Stack {
                        .children = {
                            new ui::Scale9Sprite {
                                .fileName = "GJ_button_01.png",
                                .size = CCSize{50.f, 30.f},
                            },
                            new ui::TextArea {
                                .text = "Add",
                                .font = "bigFont.fnt",
                                .scale = 0.6f,
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
                .id = "people-scroll-layer"_spr,
                .count = m_setting->users.size(),
                .builder = [=, this](auto idx) -> ui::Base* {
                    return this->generatePersonEntry(m_setting->users[idx].name, m_setting->users[idx].role);
                },
            },
        },
    };

    auto generalAccessType = new ui::Row {
        .store = &m_generalAccessTypeRow,
        .id = "general-access-type-row"_spr,
        .children = {
            new ui::MenuItemSpriteExtra {
                .id = "general-access-type-change-button-left"_spr,
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
                .id = "general-access-type-text"_spr,
                .text = "",
                .font = "bigFont.fnt",
                .scale = 0.45f,
            },
            new ui::Container {.width = 4},
            new ui::MenuItemSpriteExtra {
                .id = "general-access-type-change-button-right"_spr,
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
        .id = "general-access-change-column"_spr,
        .crossAxis = ui::CrossAxisAlignment::Start,
        .children = {
            new ui::Row {
                .id = "general-access-change-row"_spr,
                .children = {
                    new ui::MenuItemSpriteExtra {
                        .id = "general-access-change-button-left"_spr,
                        .callback = [this](auto*){
                            m_setting->discoverable = !m_setting->discoverable;
                            if (m_setting->discoverable) {
                                m_setting->defaultSharing = DefaultSharingType::Viewer;
                            }
                            else {
                                m_setting->defaultSharing = DefaultSharingType::Restricted;
                            }
                            this->updateValues();
                        },
                        .child = new ui::Sprite {
                            .frameName = "GJ_arrow_01_001.png",
                            .scale = 0.35f
                        },
                    },
                    new ui::Container {.width = 4},
                    new ui::TextArea {
                        .store = reinterpret_cast<CCNode**>(&m_generalAccessText),
                        .id = "general-access-text"_spr,
                        .text = "",
                        .font = "bigFont.fnt",
                        .scale = 0.35f,
                    },
                    new ui::Container {.width = 4},
                    new ui::MenuItemSpriteExtra {
                        .id = "general-access-change-button-right"_spr,
                        .callback = [this](auto*){
                            m_setting->discoverable = !m_setting->discoverable;
                            if (m_setting->discoverable) {
                                m_setting->defaultSharing = DefaultSharingType::Viewer;
                            }
                            else {
                                m_setting->defaultSharing = DefaultSharingType::Restricted;
                            }
                            this->updateValues();
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
                .id = "general-access-description"_spr,
                .text = "",
                .font = "chatFont.fnt",
                .scale = 0.55f,
            },
        },
    };

    auto gen = new ui::Popup {
        .size = POPUP_SIZE,
        .id = "share-settings-popup"_spr,
        .bgId = "share-settings-bg"_spr,
        .closeId = "share-settings-close"_spr,
        .child = new ui::Center {
            .store = reinterpret_cast<CCNode**>(&m_center),
            .id = "share-settings-center"_spr,
            .child = new ui::Container {
                .id = "share-settings-container"_spr,
                .padding = ui::EdgeInsets::Symmetric{.horizontal = 20.f, .vertical = 10.f},
                .child = new ui::Column {
                    .id = "share-settings-column"_spr,
                    .children = {
                        new ui::Row {
                            .id = "share-settings-row"_spr,
                            .mainAxis = ui::MainAxisAlignment::Center,
                            .children = {
                                new ui::TextArea {
                                    .id = "share-settings-title"_spr,
                                    .text = fmt::format("Share \"{}\"", m_setting->title),
                                    .font = "bigFont.fnt",
                                    .scale = 0.7f,
                                },
                            },
                        },
                        new ui::Container {.height = 10},
                        shareWith,
                        new ui::Container {.height = 10},
                        new ui::TextArea {
                            .id = "people-with-access-title"_spr,
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
                            .id = "general-access-title"_spr,
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
                                        .id = "general-access-row"_spr,
                                        .children = {
                                            new ui::Sprite {
                                                .id = "general-access-icon"_spr,
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
        .id = name + "-entry"_spr,
        .child = new ui::Menu {
            .id = name + "-menu"_spr,
            .child = new ui::Row {
                .id = name + "-row"_spr,
                .children = {
                    new ui::TextArea {
                        .id = name + "-access-text"_spr,
                        .text = std::string(name),
                        .font = "bigFont.fnt",
                        .scale = 0.50f,
                    },
                    new ui::Expanded {},
                    new ui::MenuItemSpriteExtra {
                        .id = name + "-change-button-left"_spr,
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
                            if (auto label = static_cast<geode::SimpleTextArea*>(m_peopleScrollLayer->getChildByIDRecursive(name + "-sharing-text"_spr))) {
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
                        .id = name + "-sharing-text"_spr,
                        .text = this->getSharingTypeString(type),
                        .font = "bigFont.fnt",
                        .scale = 0.45f,
                    },
                    new ui::Container {.width = 4},
                    new ui::MenuItemSpriteExtra {
                        .id = name + "-change-button-right"_spr,
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
                            if (auto label = static_cast<geode::SimpleTextArea*>(m_peopleScrollLayer->getChildByIDRecursive(name + "-sharing-text"_spr))) {
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
                    new ui::Container {.width = 8},
                    new ui::MenuItemSpriteExtra {
                        .id = name + "-delete-button"_spr,
                        .callback = [this, name](auto*){
                            m_setting->removeUser(name);
                            if (auto child = m_peopleScrollLayer->getChildByIDRecursive(name + "-entry"_spr)) {
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

// ui::Base* ShareSettings::generateList() {
//     std::vector<std::pair<std::string, DefaultSharingType>> userValues;

//     for (auto& viewers : m_setting->viewers) {
//         userValues.emplace_back(viewers, DefaultSharingType::Viewer);
//     }

//     for (auto& editors : m_setting->editors) {
//         userValues.emplace_back(editors, DefaultSharingType::Editor);
//     }

//     for (auto& admins : m_setting->admins) {
//         userValues.emplace_back(admins, DefaultSharingType::Admin);
//     }

//     return new ui::ScrollLayer {
//         .store = reinterpret_cast<CCNode**>(&m_list),
//         .count = userValues.size(),
//         .builder = [=, this](auto idx) -> ui::Base* {
//             return new ui::LayerColor {
//                 .color = idx % 2 == 0 ? ccc3(0xa1, 0x58, 0x2c) : ccc3(0xc2, 0x72, 0x3e),
//                 .height = 40.f,
//                 .child = new ui::Container {
//                     .padding = ui::EdgeInsets::All{4.f},
//                     .child = new ui::Row {
//                         .mainAxis = ui::MainAxisAlignment::Between,
//                         .children = {
//                             new ui::TextArea {
//                                 .text = userValues[idx].first,
//                                 .color = ccc3(0xff, 0xff, 0xff),
//                             },
//                             new ui::TextArea {
//                                 .text = matjson::Value(userValues[idx].second).dump(),
//                                 .color = ccc3(0xff, 0xff, 0xff),
//                             },
//                         },
//                     },
//                 },
//             };
//         },
//     };
// }