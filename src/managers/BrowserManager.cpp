#include <managers/BrowserManager.hpp>
#include <data/LevelEntry.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class BrowserManager::Impl {
public:
    std::unordered_map<GJGameLevel*, LevelEntry> m_levelEntries;
    std::vector<Ref<GJGameLevel>> m_myLevels;
    std::vector<Ref<GJGameLevel>> m_sharedLevels;
    std::vector<Ref<GJGameLevel>> m_discoverLevels;

    cocos2d::CCArray* getMyLevels(int folder);
    cocos2d::CCArray* getSharedLevels();
    cocos2d::CCArray* getDiscoverLevels();

    std::optional<std::string> getLevelKey(GJGameLevel* level);

    void updateMyLevels(std::vector<LevelEntry>&& entries);
    void updateSharedLevels(std::vector<LevelEntry>&& entries);
    void updateDiscoverLevels(std::vector<LevelEntry>&& entries);
};

cocos2d::CCArray* BrowserManager::Impl::getMyLevels(int folder) {
    auto arr = cocos2d::CCArray::create();
    if (folder == 0) for (auto& level : m_myLevels) {
        arr->addObject(level);
    }
    for (auto& level : CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels)) {
        if (level->m_levelFolder == folder) arr->addObject(level);
    }
    return arr;
}

cocos2d::CCArray* BrowserManager::Impl::getSharedLevels() {
    auto arr = cocos2d::CCArray::create();
    for (auto& level : m_sharedLevels) {
        arr->addObject(level);
    }
    return arr;
}

cocos2d::CCArray* BrowserManager::Impl::getDiscoverLevels() {
    auto arr = cocos2d::CCArray::create();
    for (auto& level : m_discoverLevels) {
        arr->addObject(level);
    }
    return arr;
}

std::optional<std::string> BrowserManager::Impl::getLevelKey(GJGameLevel* level) {
    auto it = m_levelEntries.find(level);
    if (it == m_levelEntries.end()) {
        return std::nullopt;
    }
    return it->second.key;
}

void BrowserManager::Impl::updateMyLevels(std::vector<LevelEntry>&& entries) {
    for (auto const& level : m_myLevels) {
        m_levelEntries.erase(level);
    }
    m_myLevels.clear();

    for (auto& entry : entries) {
        auto localLevels = CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels);

        if (auto it = std::find_if(localLevels.begin(), localLevels.end(), [&](auto& localLevel) {
            return EditorIDs::getID(localLevel) == entry.uniqueId;
        }); it != localLevels.end()) {
            m_myLevels.push_back(*it);
            m_levelEntries[*it] = std::move(entry);
        }
        else {
            auto level = GJGameLevel::create();
            level->m_levelType = GJLevelType::Editor;
            level->m_levelName = entry.settings.title;
            level->m_levelDesc = entry.settings.description;
            m_myLevels.push_back(level);
            m_levelEntries[level] = std::move(entry);
        }
    }
}

void BrowserManager::Impl::updateSharedLevels(std::vector<LevelEntry>&& entries) {
    for (auto const& level : m_sharedLevels) {
        m_levelEntries.erase(level);
    }
    m_sharedLevels.clear();

    for (auto& entry : entries) {
        auto level = GJGameLevel::create();
        level->m_levelName = entry.settings.title;
        level->m_levelDesc = entry.settings.description;
        m_sharedLevels.push_back(level);
        m_levelEntries[level] = std::move(entry);
    }
}

void BrowserManager::Impl::updateDiscoverLevels(std::vector<LevelEntry>&& entries) {
    for (auto const& level : m_discoverLevels) {
        m_levelEntries.erase(level);
    }
    m_discoverLevels.clear();

    for (auto& entry : entries) {
        auto level = GJGameLevel::create();
        level->m_levelName = entry.settings.title;
        level->m_levelDesc = entry.settings.description;
        m_discoverLevels.push_back(level);
        m_levelEntries[level] = std::move(entry);
    }
}

BrowserManager* BrowserManager::get() {
    static BrowserManager instance;
    return &instance;
}

BrowserManager::BrowserManager() : impl(std::make_unique<Impl>()) {}
BrowserManager::~BrowserManager() = default;

cocos2d::CCArray* BrowserManager::getMyLevels(int folder) {
    return impl->getMyLevels(folder);
}

cocos2d::CCArray* BrowserManager::getSharedLevels() {
    return impl->getSharedLevels();
}

cocos2d::CCArray* BrowserManager::getDiscoverLevels() {
    return impl->getDiscoverLevels();
}

std::optional<std::string> BrowserManager::getLevelKey(GJGameLevel* level) {
    return impl->getLevelKey(level);
}

void BrowserManager::updateMyLevels(std::vector<LevelEntry>&& entries) {
    impl->updateMyLevels(std::move(entries));
}

void BrowserManager::updateSharedLevels(std::vector<LevelEntry>&& entries) {
    impl->updateSharedLevels(std::move(entries));
}

void BrowserManager::updateDiscoverLevels(std::vector<LevelEntry>&& entries) {
    impl->updateDiscoverLevels(std::move(entries));
}