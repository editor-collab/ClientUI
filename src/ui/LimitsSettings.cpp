#include <ui/LimitsSettings.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/LevelManager.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

static constexpr auto POPUP_SIZE = CCSize{400.f, 300.f};

LimitsSettings* LimitsSettings::create(std::string_view key, LevelSetting* setting, SettingUserEntry* entry) {
    auto ret = new (std::nothrow) LimitsSettings();
    if (ret && ret->init(key, setting, entry)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LimitsSettings::init(std::string_view key, LevelSetting* setting, SettingUserEntry* entry) {
    if (!CCNode::init()) return false;

    m_key = key;
    m_entry = entry;
    m_setting = setting;

    auto gen = new ui::Popup {
        .size = POPUP_SIZE,
        .id = "limits-settings-popup"_spr,
        .bgId = "limits-settings-bg"_spr,
        .closeId = "limits-settings-close"_spr,
        .child = new ui::Center {
            .id = "limits-settings-center"_spr,
            .child = new ui::Container {
                .id = "limits-settings-container"_spr,
                .padding = ui::EdgeInsets::Symmetric{.horizontal = 20.f, .vertical = 10.f},
                .child = new ui::Column {
                    .id = "limits-settings-column"_spr,
                    .children = {
                        new ui::Row {
                            .id = "limits-settings-row"_spr,
                            .mainAxis = ui::MainAxisAlignment::Center,
                            .children = {
                                new ui::TextArea {
                                    .id = "limits-settings-title"_spr,
                                    .text = fmt::format("Limits for \"{}\"", m_entry->name),
                                    .font = "bigFont.fnt",
                                    .scale = 0.7f,
                                },
                            },
                        },
                        new ui::Container {.height = 5},
                        new ui::Container {
                            .height = 75,
                            .child = new ui::Row {
                                .children = {
                                    new ui::Expanded {
                                        .child = this->generateGrid(&m_groupsGrid, "Group IDs", "groups", &m_entry->limits.groups),
                                    },
                                    new ui::Expanded {
                                        .child = this->generateGrid(&m_colorsGrid, "Color IDs", "colors", &m_entry->limits.colors),
                                    },
                                },
                            },
                        },
                        new ui::Container {
                            .height = 75,
                            .child = new ui::Row {
                                .children = {
                                    new ui::Expanded {
                                        .child = this->generateGrid(&m_itemsGrid, "Item/Block IDs", "items", &m_entry->limits.items),
                                    },
                                    new ui::Expanded {
                                        .child = this->generateGrid(&m_layersGrid, "Layers", "layers", &m_entry->limits.layers),
                                    },
                                },
                            },
                        },
                        new ui::Container {
                            .height = 75,
                            .child = new ui::Row {
                                .children = {
                                    new ui::Expanded {
                                        .child = new ui::Row {
                                            .children = {
                                                new ui::Expanded {
                                                    .child = this->generateInput(&m_minRangeInput, "Min", "min-range"),
                                                },
                                                new ui::Container {.width = 5},
                                                new ui::Expanded {
                                                    .child = this->generateInput(&m_maxRangeInput, "Max", "max-range"),
                                                },                                                
                                            },
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

ui::Base* LimitsSettings::generateGrid(cocos2d::CCNode** node, std::string name, std::string id, std::vector<AllowedRange>* range) {
    return new ui::Container {
        .id = id + "-main-container",
        .padding = ui::EdgeInsets{
            .bottom = 5.f,
            .left = 5.f,
            .right = 5.f,
        },
        .child = new ui::Column {
            .id = id + "-main-column",
            .children = {
                new ui::Row {
                    .id = id + "-title-row",
                    .children = {
                        new ui::TextArea {
                            .id = id + "-title-text",
                            .text = name,
                            .scale = .45f,
                            .font = "bigFont.fnt",
                        },
                        new ui::Expanded {},
                        new ui::Menu {
                            .id = id + "-title-menu",
                            .child = new ui::MenuItemSpriteExtra {
                                .id = id + "-add-button",
                                .callback = [=, this](auto*) {
                                    auto min = numFromString<uint32_t>(m_minRangeInput->getString()).unwrapOr(0);
                                    auto max = numFromString<uint32_t>(m_maxRangeInput->getString()).unwrapOr(0);
                                    if (min > max) std::swap(min, max);
                                    if (min == 0 || max == 0) return;
                                    range->push_back(AllowedRange{min, max});
                                    this->updateValues();
                                },
                                .child = new ui::Scale9Sprite {
                                    .id = id + "-add-sprite",
                                    .scale = .5f,
                                    .fileName = "GJ_button_01.png",
                                    .child = new ui::Container {
                                        .id = id + "-add-padding",
                                        .padding = ui::EdgeInsets::All{3.f},
                                        .child = new ui::TextArea {
                                            .id = id + "-add-text",
                                            .text = "Add",
                                            .scale = .4f,
                                            .font = "bigFont.fnt",
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
                new ui::Container {.height = 5.f},
                new ui::Expanded {
                    .child = new ui::Scale9Sprite {
                        .id = id + "-background",
                        .fileName = "square02b_001.png",
                        .scale = .5f,
                        .color = ccc4(0x00, 0x00, 0x00, 0x5a),
                        .child = new ui::Menu {
                            .id = id + "-menu",
                            .child = new ui::Container {
                                .id = id + "-padding",
                                .padding = ui::EdgeInsets::All{3.f},
                                .child = new ui::Grid {
                                    .store = node,
                                    .id = id + "-grid",
                                    .axis = ui::Axis::Vertical,
                                    .childAspectRatio = .5f,
                                    .crossAxisCount = 3,
                                },
                            },
                        },
                    },
                },
            },
        },
    };
}

void LimitsSettings::updateValues() {
    auto fillGrid = [&](auto* grid, auto* range, auto id) {
        grid->removeAllChildren();
        for (size_t i = 0; i < range->size(); i++) {
            grid->addChild(this->generateItem(i, id, range)->get());
        }
        grid->updateLayout();
    };

    fillGrid(m_layersGrid, &m_entry->limits.layers, "layers");
    fillGrid(m_groupsGrid, &m_entry->limits.groups, "groups");
    fillGrid(m_colorsGrid, &m_entry->limits.colors, "colors");
    fillGrid(m_itemsGrid, &m_entry->limits.items, "items");

    auto task = LevelManager::get()->updateLevelSettings(
        m_key,
        *m_setting
    );
    task.listen([=](auto* result) {});
}

ui::Base* LimitsSettings::generateItem(size_t idx, std::string id, std::vector<AllowedRange>* range) {
    return new ui::Container {
        .id = fmt::format("{}-{}-container", id, idx),
        .padding = ui::EdgeInsets::All{3.f},
        .child = new ui::MenuItemSpriteExtra {
            .id = fmt::format("{}-{}-button", id, idx),
            .callback = [=, this](auto*) {
                range->erase(range->begin() + idx);
                this->updateValues();
            },
            .child = new ui::Scale9Sprite {
                .id = fmt::format("{}-{}-sprite", id, idx),
                .fileName = "GJ_button_04.png",
                .scale = .5f,
                .child = new ui::Container {
                    .id = fmt::format("{}-{}-padding", id, idx),
                    .padding = ui::EdgeInsets::All{3.f},
                    .child = new ui::TextArea {
                        .id = fmt::format("{}-{}-text", id, idx),
                        .text = fmt::format("{}-{}", range->at(idx).min, range->at(idx).max),
                        .scale = .25f,
                        .alignment = ui::TextAlignment::Center,
                        .font = "bigFont.fnt",
                    },
                },
            },
        },
    };
}

ui::Base* LimitsSettings::generateInput(geode::TextInput** input, std::string name, std::string id) {
    return new ui::Container {
        .id = id + "-main-container",
        .padding = ui::EdgeInsets{
            .bottom = 5.f,
            .left = 5.f,
            .right = 5.f,
        },
        .child = new ui::Column {
            .children = {
                new ui::Menu {
                    .child = new ui::Row {
                        .children = {
                            new ui::MenuItemSpriteExtra {
                                .id = id + "-left-button",
                                .callback = [=](auto*) {
                                    auto value = numFromString<int>((*input)->getString()).unwrapOr(0);
                                    value -= 1;
                                    value = std::clamp<int>(value, 1, 9999);
                                    (*input)->setString(std::to_string(value));
                                },
                                .child = new ui::Sprite {
                                    .frameName = "GJ_arrow_01_001.png",
                                    .scaleX = 0.55f,
                                    .scaleY = 0.55f,
                                },
                            },
                            new ui::Container {.width = 5.f},
                            new ui::Expanded {
                                .child = new ui::TextInput {
                                    .store = reinterpret_cast<CCNode**>(input),
                                    .placeholder = name,
                                    .filter = geode::CommonFilter::Uint,
                                    .maxCharCount = 4,
                                },
                            },
                            new ui::Container {.width = 5.f},
                            new ui::MenuItemSpriteExtra {
                                .id = id + "-right-button",
                                .callback = [=](auto*) {
                                    auto value = numFromString<int>((*input)->getString()).unwrapOr(0);
                                    value += 1;
                                    value = std::clamp<int>(value, 1, 9999);
                                    (*input)->setString(std::to_string(value));
                                },
                                .child = new ui::Sprite {
                                    .frameName = "GJ_arrow_01_001.png",
                                    .scaleX = -0.55f,
                                    .scaleY = 0.55f,
                                },
                            },
                        },
                    },
                },
            },
        },
    };
}