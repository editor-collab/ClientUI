#include <hooks/ui/LevelBrowserLayer.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <managers/BrowserManager.hpp>
#include <managers/CellManager.hpp>
#include <lavender/Lavender.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

template <class Lambda>
CCMenuItemSpriteExtra* LevelBrowserLayerUIHook::generateTabButton(std::string_view framename, std::string_view id, Lambda&& func) {
    auto button = CCMenuItemExt::createSpriteExtraWithFrameName(framename, 1.f, func);
    button->m_colorEnabled = true;
    button->m_animationEnabled = false;
    button->m_colorDip = 0.80f;
    button->setID(id.data());
    return button;
}

CCSprite* LevelBrowserLayerUIHook::generateTabSprite(std::string_view framename, std::string_view id, bool visible) {
    auto button = CCSprite::createWithSpriteFrameName(framename.data());
    button->setID(id.data());
    button->setVisible(visible);
    return button;
}

void LevelBrowserLayerUIHook::revisualizeButtons(CCObject* sender) {
    for (auto child : CCArrayExt<CCNode*>(m_fields->onTabMenu->getChildren())) {
        child->setVisible(false);
    }
    for (auto child : CCArrayExt<CCNode*>(m_fields->offTabMenu->getChildren())) {
        child->setVisible(true);
    }
    if (sender) {
        static_cast<CCNode*>(sender)->setVisible(false);
    }
}

void LevelBrowserLayerUIHook::onMyLevels(CCObject* sender) {
    this->revisualizeButtons(sender);
    m_fields->onTabMenu->getChildByID("my-levels-tab-on"_spr)->setVisible(true);
    m_fields->currentTab = CurrentTab::MyLevels;
    m_fields->myLevelsListener.setFilter(FetchManager::get()->getMyLevels());
}
void LevelBrowserLayerUIHook::onSharedWithMe(CCObject* sender) {
    this->revisualizeButtons(sender);
    m_fields->onTabMenu->getChildByID("shared-with-me-tab-on"_spr)->setVisible(true);
    m_fields->currentTab = CurrentTab::SharedWithMe;
    m_fields->sharedWithMeListener.setFilter(FetchManager::get()->getSharedWithMe());
}
void LevelBrowserLayerUIHook::onDiscover(CCObject* sender) {
    this->revisualizeButtons(sender);
    m_fields->onTabMenu->getChildByID("discover-tab-on"_spr)->setVisible(true);
    m_fields->currentTab = CurrentTab::Discover;
    m_fields->discoverListener.setFilter(FetchManager::get()->getDiscover());
}

$override
void LevelBrowserLayerUIHook::setupLevelBrowser(cocos2d::CCArray* items) {
    LevelBrowserLayer::setupLevelBrowser(items);

    log::debug("items: {}", items);

    if (!m_searchObject || m_searchObject->m_searchType != SearchType::MyLevels) return;

    if (auto top = static_cast<CCSprite*>(m_list->getChildByID("top-border"))) {
        m_list->getChildByID("title")->setVisible(false);
        top->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName("GJ_table_top02_001.png"));

        m_fields->offTabMenu = CCMenu::create();
        m_fields->offTabMenu->setContentSize(top->getContentSize() - ccp(60, 0));
        m_fields->offTabMenu->setAnchorPoint(top->getAnchorPoint());
        m_fields->offTabMenu->setPosition(top->getPosition() + ccp(0, 11));
        m_fields->offTabMenu->setLayout(RowLayout::create()->setAutoScale(false)->setAxisAlignment(AxisAlignment::Between));

        m_fields->onTabMenu = CCMenu::create();
        m_fields->onTabMenu->setContentSize(top->getContentSize() - ccp(60, 0));
        m_fields->onTabMenu->setAnchorPoint(top->getAnchorPoint());
        m_fields->onTabMenu->setPosition(top->getPosition() + ccp(0, 11));
        m_fields->onTabMenu->setLayout(RowLayout::create()->setAutoScale(false)->setAxisAlignment(AxisAlignment::Between));

        m_fields->offTabMenu->addChild(this->generateTabButton("MyLevelsTabOff.png"_spr, "my-levels-tab-off"_spr, [=, this](auto* sender) {
            this->onMyLevels(sender);
        }));
        m_fields->offTabMenu->addChild(this->generateTabButton("SharedWithMeTabOff.png"_spr, "shared-with-me-tab-off"_spr, [=, this](auto* sender) {
            this->onSharedWithMe(sender);
        }));
        m_fields->offTabMenu->addChild(this->generateTabButton("DiscoverTabOff.png"_spr, "discover-tab-off"_spr, [=, this](auto* sender) {
            this->onDiscover(sender);
        }));

        m_fields->onTabMenu->addChild(this->generateTabSprite("MyLevelsTabOn.png"_spr, "my-levels-tab-on"_spr, m_fields->currentTab == CurrentTab::MyLevels));
        m_fields->onTabMenu->addChild(this->generateTabSprite("SharedWithMeTabOn.png"_spr, "shared-with-me-tab-on"_spr, m_fields->currentTab == CurrentTab::SharedWithMe));
        m_fields->onTabMenu->addChild(this->generateTabSprite("DiscoverTabOn.png"_spr, "discover-tab-on"_spr, m_fields->currentTab == CurrentTab::Discover));

        m_fields->offTabMenu->updateLayout();
        m_fields->offTabMenu->setID("tab-off-menu"_spr);

        m_list->addChild(m_fields->offTabMenu, -15);

        m_fields->onTabMenu->updateLayout();
        m_fields->onTabMenu->setID("tab-on-menu"_spr);

        m_list->addChild(m_fields->onTabMenu, 15);
    }

    for (auto cell : CCArrayExt<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray)) {
        if (BrowserManager::get()->isMyLevel(cell->m_level)) {
            CellManager::get()->applyMyLevel(cell, *BrowserManager::get()->getLevelEntry(cell->m_level));
        }
        else if (BrowserManager::get()->isSharedLevel(cell->m_level)) {
            CellManager::get()->applySharedLevel(cell, *BrowserManager::get()->getLevelEntry(cell->m_level));
        }
        else if (BrowserManager::get()->isDiscoverLevel(cell->m_level)) {
            CellManager::get()->applyDiscoverLevel(cell, *BrowserManager::get()->getLevelEntry(cell->m_level));
        }
    }
}

$override
void LevelBrowserLayerUIHook::loadLevelsFinished(CCArray* levels, char const* ident, int searchType) {
    log::debug("levels {}", levels);
    log::debug("search object {}", m_searchObject);
    if (!m_searchObject || m_searchObject->m_searchType != SearchType::MyLevels) return LevelBrowserLayer::loadLevelsFinished(levels, ident, searchType);
    auto const page = m_searchObject->m_page;
    CCArray* fullLevels;

    log::debug("page: {}", page);

    switch (m_fields->currentTab) {
        case CurrentTab::MyLevels:
            fullLevels = BrowserManager::get()->getMyLevels(m_searchObject->m_folder);
            break;
        case CurrentTab::SharedWithMe:
            fullLevels = BrowserManager::get()->getSharedLevels();
            break;
        case CurrentTab::Discover:
            fullLevels = BrowserManager::get()->getDiscoverLevels();
            break;
    }

    constexpr auto levelsPerPage = 10;
    auto const start = page * levelsPerPage;
    auto const end = std::min<int>(start + levelsPerPage, fullLevels->count());
    auto newLevels = CCArray::create();
    for (auto i = start; i < end; i++) {
        newLevels->addObject(fullLevels->objectAtIndex(i));
    }

    log::debug("newLevels {}", newLevels);

    LevelBrowserLayer::loadLevelsFinished(newLevels, ident, searchType);
}

// void LevelBrowserLayerUIHook::updateMyLevelCells(std::vector<LevelEntry>&& entries) {
//     auto levels = CCArrayExt<GJGameLevel*>(m_levels);
//     m_fields->missingMyLevels.clear();
//     for (auto const& entry : entries) {
//         if (auto levelp = std::find_if(levels.begin(), levels.end(), [=](auto const& level) {
//             return EditorIDs::getID(level) == entry.uniqueId;
//         }); levelp != levels.end()) {
//             log::debug("Updating level for level {}", entry.uniqueId);
//             (*levelp)->m_levelName = entry.settings.title;
//             (*levelp)->m_levelDesc = entry.settings.description;
//         }
//         else {
//             log::debug("Creating new level for level {}", entry.uniqueId);
//             auto level = GJGameLevel::create();
//             level->m_levelName = entry.settings.title;
//             level->m_levelDesc = entry.settings.description;
//             level->m_levelType = GJLevelType::Editor;
//             m_fields->missingMyLevels.push_back(level);
//         }
//     }
//     auto newLevels = CCArray::create();
//     for (auto level : m_fields->missingMyLevels) {
//         newLevels->addObject(level);
//     }
//     for (auto level : levels) {
//         newLevels->addObject(level);
//     }
//     this->setupLevelBrowser(newLevels);
    
//     for (auto cell : CCArrayExt<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray)) {
//         auto correspondingEntry = std::find_if(entries.begin(), entries.end(), [=](auto const& entry) {
//             return EditorIDs::getID(cell->m_level) == entry.uniqueId;
//         });
//         auto correspondingLevel = std::find_if(m_fields->missingMyLevels.begin(), m_fields->missingMyLevels.end(), [=](auto const& level) {
//             return level == cell->m_level;
//         });
//         if (correspondingEntry != entries.end() || correspondingLevel != m_fields->missingMyLevels.end()) {
//             if (auto node = cell->getChildByIDRecursive("length-icon")) node->setVisible(false);
//             if (auto node = cell->getChildByIDRecursive("length-label")) node->setVisible(false);
//             if (auto node = cell->getChildByIDRecursive("song-icon")) node->setVisible(false);
//             if (auto node = cell->getChildByIDRecursive("song-label")) node->setVisible(false);
//             if (auto node = cell->getChildByIDRecursive("info-icon")) node->setVisible(false);
//             if (auto node = cell->getChildByIDRecursive("info-label")) node->setVisible(false);
//             if (auto node = typeinfo_cast<CCLabelBMFont*>(cell->getChildByIDRecursive("level-name"))) {
//                 node->setString(("Shared: " + std::string(node->getString())).c_str());
//             }
//         }
//     }
// }

$override
bool LevelBrowserLayerUIHook::init(GJSearchObject* searchObject) {
	if (!LevelBrowserLayer::init(searchObject)) return false;

	if (searchObject->m_searchType != SearchType::MyLevels) return true;

    m_fields->myLevelsListener.bind([=, this](auto* event) {
        if (auto resultp = event->getValue(); resultp) {
            if (GEODE_UNWRAP_IF_OK(levels, *resultp)) {
                Notification::create("My levels fetched", nullptr, 1.5f)->show();
                for (auto& entry : levels) {
                    log::debug("Level: {}", entry.key);
                }
                BrowserManager::get()->updateMyLevels(std::move(levels));
                this->loadPage(m_searchObject);
            }
            else {
                Notification::create("Failed to fetch my levels", nullptr, 1.5f)->show();
            }
        }
    });

    m_fields->sharedWithMeListener.bind([=, this](auto* event) {
        if (auto resultp = event->getValue(); resultp) {
            if (GEODE_UNWRAP_IF_OK(levels, *resultp)) {
                Notification::create("Shared levels fetched", nullptr, 1.5f)->show();
                for (auto& entry : levels) {
                    log::debug("Level: {}", entry.key);
                }
                BrowserManager::get()->updateSharedLevels(std::move(levels));
                this->loadPage(m_searchObject);
            }
            else {
                Notification::create("Failed to fetch shared levels", nullptr, 1.5f)->show();
            }
        }
    });

    m_fields->discoverListener.bind([=, this](auto* event) {
        if (auto resultp = event->getValue(); resultp) {
            if (GEODE_UNWRAP_IF_OK(levels, *resultp)) {
                Notification::create("Discover levels fetched", nullptr, 1.5f)->show();
                for (auto& entry : levels) {
                    log::debug("Level: {}", entry.key);
                }
                BrowserManager::get()->updateDiscoverLevels(std::move(levels));
                this->loadPage(m_searchObject);
            }
            else {
                Notification::create("Failed to fetch discover levels", nullptr, 1.5f)->show();
            }
        }
    });
	return true;
}