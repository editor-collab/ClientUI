#include <ui/BuyPopup.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <managers/AccountManager.hpp>
#include <managers/FetchManager.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

static constexpr auto POPUP_SIZE = CCSize{420.f, 280.f};

BuyPopup* BuyPopup::create() {
    auto ret = new (std::nothrow) BuyPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BuyPopup::init() {
    if (!CCNode::init()) return false;

    ui::TypedController<geode::SimpleTextArea> textAreaController;
    ui::TypedController<geode::TextInput> textInputController;
    ui::Controller buttonController;
    ui::Controller popupController;

    auto gen = new ui::Popup {
        .size = POPUP_SIZE,
        .controller = popupController,
        .id = "buy-popup"_spr,
        .bgId = "buy-popup-bg"_spr,
        .bgFileName = std::nullopt,
        .closeId = "buy-popup-close"_spr,
        .child = new ui::Stack {
            .children = {
                new ui::Sprite {
                    .id = "buy-popup-bg"_spr,
                    .frameName = "BuyPromo.png"_spr,
                },
                new ui::Align {
                    .alignment = ui::Alignment::BottomCenter,
                    .child = new ui::Container {
                        .padding = ui::EdgeInsets::Symmetric{.vertical = 30.f, .horizontal = 50.f},
                        .child = new ui::TextInput {
                            .controller = textInputController,
                            .maxCharCount = 19,
                            .placeholder = "XXXX-XXXX-XXXX-XXXX",
                            .filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                            .callback = [=, this](auto const& str) {
                                if (str.size() > 0) {
                                    textAreaController->setText("Activate");
                                }
                                else {
                                    textAreaController->setText("Buy");
                                }
                                buttonController->updateLayout();

                                if (str.size() > m_text.size()) {
                                    if (str.size() == 4 || str.size() == 9 || str.size() == 14) {
                                        m_text = str + "-";
                                    }
                                    else m_text = str;
                                }
                                else {
                                    if (str.size() == 4 || str.size() == 9 || str.size() == 14) {
                                        m_text = str.substr(0, str.size() - 1);
                                    }
                                    else m_text = str;
                                }
                                textInputController->setString(m_text);
                            },
                        },
                    },
                },
                new ui::Menu {
                    .child = new ui::Align {
                        .alignment = ui::Alignment::BottomCenter,
                        .child = new ui::Transform {
                            .offset = ccp(0.f, 0.f),
                            .anchor = ccp(0.5f, 1.f),
                            .child = new ui::MenuItemSpriteExtra {
                                .controller = buttonController,
                                .callback = [=, this](auto* self) {
                                    if (m_text.size() == 19) {
                                        auto key = m_text.substr(0, 4) + m_text.substr(5, 4) + m_text.substr(10, 4) + m_text.substr(15, 4);
                                        auto task = AccountManager::get()->claimKey(key);
                                        task.listen([=, this](auto* resultp) {
                                            if (GEODE_UNWRAP_EITHER(value, err, *resultp)) {
                                                popupController->removeFromParentAndCleanup(true);
                                                FetchManager::get()->addHostableCount(value);
                                                createQuickPopup("Success", "Key claimed successfully!", "Cancel", "OK", [=](auto, auto) {});
                                            }
                                            else {
                                                log::warn("Claim key error: {}", err);
                                                createQuickPopup("Error", "Could not claim the key", "Cancel", "OK", [](auto, auto) {});
                                            }
                                        });
                                    }
                                    else if (m_text.size() == 0) {
                                        web::openLinkInBrowser("https://buy.stripe.com/aEUbLb38R2Cw91K9AA");
                                    }
                                },
                                .child = new ui::Scale9Sprite {
                                    .fileName = "GJ_button_03.png",
                                    .child = new ui::Container {
                                        .padding = ui::EdgeInsets::All{7.f},
                                        .child = new ui::TextArea {
                                            .controller = textAreaController,
                                            .text = "Buy",
                                            .font = "bigFont.fnt",
                                            .scale = 0.7f,
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

    return true;
}

void BuyPopup::update(float dt) {

}