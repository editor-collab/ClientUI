#include <managers/BrowserManager.hpp>
#include <data/LevelEntry.hpp>
#include <utils/CryptoHelper.hpp>
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

    bool isMyLevel(GJGameLevel* level);
    bool isSharedLevel(GJGameLevel* level);
    bool isDiscoverLevel(GJGameLevel* level);

    void setLevelValues(GJGameLevel* level, LevelEntry const& entry);

    std::optional<std::string> getLevelKey(GJGameLevel* level);
    std::optional<LevelEntry*> getLevelEntry(GJGameLevel* level);
    std::optional<LevelEntry*> getLevelEntry(std::string_view key);

    void updateMyLevels(std::vector<LevelEntry>&& entries);
    void updateSharedLevels(std::vector<LevelEntry>&& entries);
    void updateDiscoverLevels(std::vector<LevelEntry>&& entries);
};

cocos2d::CCArray* BrowserManager::Impl::getMyLevels(int folder) {
    auto arr = cocos2d::CCArray::create();
    auto localLevels = CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels);
    if (folder == 0) {
        for (auto& level : m_myLevels) {
            if (std::find(localLevels.begin(), localLevels.end(), level) == localLevels.end()) arr->addObject(level);
        }
    }
    for (auto& level : localLevels) {
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

bool BrowserManager::Impl::isMyLevel(GJGameLevel* level) {
    return std::find(m_myLevels.begin(), m_myLevels.end(), level) != m_myLevels.end();
}

bool BrowserManager::Impl::isSharedLevel(GJGameLevel* level) {
    return std::find(m_sharedLevels.begin(), m_sharedLevels.end(), level) != m_sharedLevels.end();
}

bool BrowserManager::Impl::isDiscoverLevel(GJGameLevel* level) {
    return std::find(m_discoverLevels.begin(), m_discoverLevels.end(), level) != m_discoverLevels.end();
}

std::optional<std::string> BrowserManager::Impl::getLevelKey(GJGameLevel* level) {
    auto it = m_levelEntries.find(level);
    if (it == m_levelEntries.end()) {
        return std::nullopt;
    }
    return it->second.key;
}

std::optional<LevelEntry*> BrowserManager::Impl::getLevelEntry(GJGameLevel* level) {
    auto it = m_levelEntries.find(level);
    if (it == m_levelEntries.end()) {
        return std::nullopt;
    }
    return &it->second;
}

std::optional<LevelEntry*> BrowserManager::Impl::getLevelEntry(std::string_view key) {
    for (auto& [level, entry] : m_levelEntries) {
        if (entry.key == key) {
            return &entry;
        }
    }
    return std::nullopt;
}

void BrowserManager::Impl::setLevelValues(GJGameLevel* level, LevelEntry const& entry) {
    level->m_levelType = GJLevelType::Editor;
    level->m_levelName = entry.settings.title;
    std::vector<uint8_t> data(entry.settings.description.begin(), entry.settings.description.end());
    level->m_levelDesc = crypto::base64Encode(data, crypto::Base64Flags::UrlSafe);
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
            this->setLevelValues(level, entry);
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
        this->setLevelValues(level, entry);
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
        this->setLevelValues(level, entry);
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

bool BrowserManager::isMyLevel(GJGameLevel* level) {
    return impl->isMyLevel(level);
}

bool BrowserManager::isSharedLevel(GJGameLevel* level) {
    return impl->isSharedLevel(level);
}

bool BrowserManager::isDiscoverLevel(GJGameLevel* level) {
    return impl->isDiscoverLevel(level);
}

std::optional<std::string> BrowserManager::getLevelKey(GJGameLevel* level) {
    return impl->getLevelKey(level);
}

std::optional<LevelEntry*> BrowserManager::getLevelEntry(GJGameLevel* level) {
    return impl->getLevelEntry(level);
}

std::optional<LevelEntry*> BrowserManager::getLevelEntry(std::string_view key) {
    return impl->getLevelEntry(key);
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