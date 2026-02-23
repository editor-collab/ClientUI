#include <ui/LevelUserList.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <lavender/Lavender.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Dispatch.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

static constexpr auto POPUP_SIZE = CCSize{400.f, 290.f};

LevelUserList* LevelUserList::create(LevelEntry* entry, LevelEditorLayer* editorLayer) {
    auto ret = new (std::nothrow) LevelUserList();
    if (ret && ret->init(entry, editorLayer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LevelUserList::init(LevelEntry* entry, LevelEditorLayer* editorLayer) {
    if (!CCNode::init()) return false;

    if (Mod::get()->getSavedValue<bool>("shown-user-list-popup-tutorial") == false) {
        auto popup = geode::createQuickPopup(
            "Editor Collab", 
            "Here you can find the <ca>list of people</c> in the level. "
            "If you're <cf>Admin</c> or <cy>Host</c>, you can <cr>kick or ban</c> them, if needed.",
            "OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, false
        );
        popup->m_scene = this;
        popup->show();
        Mod::get()->setSavedValue("shown-user-list-popup-tutorial", true);
    }

    m_entry = entry;
    m_setting = &entry->settings;
    m_editorLayer = editorLayer;

    Dispatch<ConnectedUserList*>("alk.editor-collab/get-user-list").send(&m_userList);

    m_userListHandle = Dispatch<ConnectedUserList>("alk.editor-collab/update-user-list").listen([this](ConnectedUserList userList) {
        m_userList = userList;
        this->updateUsers();
        return ListenerResult::Propagate;
    });

    std::vector<ConnectedUserEntry> users;
    for (auto& entry : m_userList.users) {
        users.push_back(entry);
    }

    auto filterUser = new ui::Menu {
        .child = new ui::TextInput {
            .id = "filter-user-input"_spr,
            .placeholder = "Filter user...",
            .callback = [this](std::string const& str) {
                m_filter = str;
                this->updateUsers();
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
                .children = {},
            },
        },
    };

    auto gen = new ui::Popup {
        .size = POPUP_SIZE,
        .id = "level-user-list-popup"_spr,
        .bgId = "level-user-list-bg"_spr,
        .closeId = "level-user-list-close"_spr,
        .child = new ui::Center {
            .store = reinterpret_cast<CCNode**>(&m_center),
            .id = "level-user-list-center"_spr,
            .child = new ui::Container {
                .id = "level-user-list-container"_spr,
                .padding = ui::EdgeInsets::Symmetric{.horizontal = 20.f, .vertical = 10.f},
                .child = new ui::Column {
                    .id = "level-user-list-column"_spr,
                    .children = {
                        new ui::Row {
                            .id = "level-user-list-row"_spr,
                            .mainAxis = ui::MainAxisAlignment::Center,
                            .children = {
                                new ui::TextArea {
                                    .id = "level-user-list-title"_spr,
                                    .text = "Users",
                                    .font = "bigFont.fnt",
                                    .scale = 0.7f,
                                },
                            },
                        },
                        new ui::Container {.height = 10},
                        filterUser,
                        new ui::Container {.height = 10},
                        new ui::Flexible {
                            .child = userList,
                        },
                    },
                },
            },
        },
    };

    m_popup = gen->get();
    m_popup->addChild(this);

    this->updateUsers();

    return true;
}

void LevelUserList::updateUsers() {
    m_peopleScrollLayer->m_contentLayer->removeAllChildren();
    auto onlineTitle = new ui::Container {
        .padding = ui::EdgeInsets::All{5.f},
        .child = new ui::TextArea {
            .id = "online-title"_spr,
            .text = "Online",
            .alignment = ui::TextAlignment::Center,
            .font = "bigFont.fnt",
            .scale = 0.55f,
        },
    };
    m_peopleScrollLayer->m_contentLayer->addChild(onlineTitle->get());

    auto onlineUsers = this->getFilteredOnline();
    for (auto entry : onlineUsers) {
        std::vector<ui::Base*> children;

        children.push_back(new ui::TextArea {
            .text = entry.user.accountName,
            .font = "bigFont.fnt",
            .scale = 0.55f,
            .color = ccc4(0xff, 0xff, 0xff, entry.hanging ? 0x96 : 0xff),
        });

        if (entry.hanging) {
            children.push_back(new ui::Container {.width = 10});

            // exclamation

            children.push_back(new ui::TextArea {
                .text = "(Disconnected)",
                .font = "bigFont.fnt",
                .scale = 0.35f,
                .color = ccc4(0xff, 0xff, 0xff, 0x96),
            });
        }

        children.push_back(new ui::Expanded {});

        auto accountName = GJAccountManager::get()->m_username;

        if (this->isUserAdminAndUp() && entry.user.accountName != accountName) {
            children.push_back(new ui::MenuItemSpriteExtra {
                .callback = [=, this](auto*) {
                    this->createReasonPopup([=, this](std::string_view reason) {
                        m_userList.erase(entry.user.accountId);

                        m_kickListener.spawn(LevelManager::get()->kickUser(m_entry->key, entry.user.accountId, std::string(reason)), [=](auto result) {});

                        this->updateUsers();
                    });
                },
                .child = new ui::Scale9Sprite {
                    .fileName = "GJ_button_04.png",
                    .child = new ui::Container {
                        .padding = ui::EdgeInsets::All{5.f},
                        .child = new ui::Sprite {
                            .frameName = "KickIcon.png"_spr,
                            .scale = 0.5f,
                        },
                    },
                },
            });

            children.push_back(new ui::Container {.width = 10});

            children.push_back(new ui::MenuItemSpriteExtra {
                .callback = [=, this](auto*) {
                    this->createReasonPopup([=, this](std::string_view reason) {
                        m_setting->setBanned(entry.user, reason);
                        m_userList.erase(entry.user.accountId);

                        m_updateLevelListener.spawn(LevelManager::get()->updateLevelSettings(
                            m_entry->key,
                            *m_setting
                        ), [=](auto result) {});

                        BrowserManager::get()->updateLevelEntry(m_editorLayer->m_level);

                        this->updateUsers();
                    });
                },
                .child = new ui::Scale9Sprite {
                    .fileName = "GJ_button_06.png",
                    .child = new ui::Container {
                        .padding = ui::EdgeInsets::All{5.f},
                        .child = new ui::Sprite {
                            .frameName = "BanIcon.png"_spr,
                            .scale = 0.5f,
                        },
                    },
                },
            });
        }

        auto online = new ui::Container {
            .padding = ui::EdgeInsets::All{5.f},
            .child = new ui::Menu {
                .child = new ui::Row {
                    .children = children,
                },
            },
        };
        m_peopleScrollLayer->m_contentLayer->addChild(online->get());
    }

    if (this->isUserAdminAndUp()) {
        auto bannedTitle = new ui::Container {
            .padding = ui::EdgeInsets::All{5.f},
            .child = new ui::TextArea {
                .id = "banned-title"_spr,
                .text = "Banned",
                .alignment = ui::TextAlignment::Center,
                .font = "bigFont.fnt",
                .scale = 0.55f,
            },
        };
        m_peopleScrollLayer->m_contentLayer->addChild(bannedTitle->get());

        auto bannedUsers = this->getFilteredBanned();
        for (auto entry : bannedUsers) {
            auto banned = new ui::Container {
                .padding = ui::EdgeInsets::All{5.f},
                .child = new ui::Menu {
                    .child = new ui::Row {
                        .children = {
                            new ui::TextArea {
                                .text = entry.user.accountName,
                                .font = "bigFont.fnt",
                                .scale = 0.55f,
                            },
                            new ui::Container {.width = 10},
                            new ui::Expanded {
                                .child = new ui::Row {
                                    .children = {
                                        new ui::Flexible {
                                            .child = new ui::FittedBox {
                                                .child = new ui::TextArea {
                                                    .text = entry.reason,
                                                    .font = "bigFont.fnt",
                                                    .scale = 0.45f,
                                                },
                                                .fit = ui::BoxFit::ScaleDown,
                                            },
                                        },
                                    },
                                },
                            },
                            new ui::Container {.width = 10},
                            new ui::MenuItemSpriteExtra {
                                .callback = [=, this](auto*) {
                                    m_setting->removeBanned(entry.user.accountId);

                                    m_updateLevelListener.spawn(LevelManager::get()->updateLevelSettings(
                                        m_entry->key,
                                        *m_setting
                                    ), [=](auto result) {});

                                    BrowserManager::get()->updateLevelEntry(m_editorLayer->m_level);

                                    this->updateUsers();
                                },
                                .child = new ui::Scale9Sprite {
                                    .fileName = "GJ_button_01.png",
                                    .child = new ui::Container {
                                        .padding = ui::EdgeInsets::All{5.f},
                                        .child = new ui::Sprite {
                                            .frameName = "UnbanIcon.png"_spr,
                                            .scale = 0.5f,
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            };
            m_peopleScrollLayer->m_contentLayer->addChild(banned->get());
        }
    }

    m_peopleScrollLayer->updateLayout();
}

std::vector<ConnectedUserEntry> LevelUserList::getFilteredOnline() {
    std::vector<ConnectedUserEntry> users;
    for (auto& entry : m_userList.users) {
        if (m_filter.empty() || entry.user.accountName.find(m_filter) != std::string::npos) {
            if (!m_setting->hasBanned(entry.user.accountId)) users.push_back(entry);
        }
    }
    return users;
}

std::vector<BannedUserEntry> LevelUserList::getFilteredBanned() {
    std::vector<BannedUserEntry> users;
    for (auto& entry : m_setting->banned) {
        if (m_filter.empty() || entry.user.accountName.find(m_filter) != std::string::npos) users.push_back(entry);
    }
    return users;
}

void LevelUserList::createReasonPopup(std::function<void(std::string_view)> callback) {
    auto reasonPopup = new ui::Popup {
        .size = CCSize{250.f, 150.f},
        .id = "reason-popup"_spr,
        .bgId = "reason-popup-bg"_spr,
        .closeId = "reason-popup-close"_spr,
        .child = new ui::Center {
            .child = new ui::Container {
                .padding = ui::EdgeInsets::Symmetric{.horizontal = 20.f, .vertical = 10.f},
                .child = new ui::Menu {
                    .child = new ui::Column {
                        .mainAxis = ui::MainAxisAlignment::Between,
                        .children = {
                            new ui::TextArea {
                                .text = "Enter reason",
                                .font = "bigFont.fnt",
                                .scale = 0.7f,
                            },
                            new ui::TextInput {
                                .id = "reason-input"_spr,
                                .placeholder = "Reason...",
                                .callback = [this](std::string const& str) {
                                    m_reasonString = str;
                                },
                            },
                            new ui::MenuItemSpriteExtra {
                                .callback = [this, callback](auto*) {
                                    m_reasonPopup->removeFromParentAndCleanup(true);
                                    m_reasonPopup = nullptr;
                                    callback(m_reasonString);
                                },
                                .child = new ui::Scale9Sprite {
                                    .fileName = "GJ_button_01.png",
                                    .child = new ui::Container {
                                        .padding = ui::EdgeInsets::All{5.f},
                                        .child = new ui::TextArea {
                                            .text = "OK",
                                            .font = "bigFont.fnt",
                                            .scale = 0.5f,
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

    m_reasonPopup = reasonPopup->get();
    m_reasonString = "";
}

bool LevelUserList::isUserAdminAndUp() {
    if (m_setting->getUserType(GJAccountManager::get()->m_username) == DefaultSharingType::Admin) return true;
    if (GJAccountManager::get()->m_accountID == m_entry->hostAccountId) return true;
    return false;
}