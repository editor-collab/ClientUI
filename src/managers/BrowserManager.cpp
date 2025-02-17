#include <managers/BrowserManager.hpp>
#include <managers/LocalManager.hpp>
#include <data/LevelEntry.hpp>
#include <utils/CryptoHelper.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class BrowserManager::Impl {
public:
    std::unordered_map<GJGameLevel*, LevelEntry> m_levelEntries;
    std::unordered_map<GJGameLevel*, GJGameLevel*> m_shadowEntries;
    std::vector<Ref<GJGameLevel>> m_myLevels;
    std::vector<Ref<GJGameLevel>> m_shadowMyLevels;
    std::vector<Ref<GJGameLevel>> m_sharedLevels;
    std::vector<Ref<GJGameLevel>> m_discoverLevels;

    void init();

    cocos2d::CCArray* getShadowMyLevels();
    cocos2d::CCArray* getLocalLevels(int folder);
    cocos2d::CCArray* getMyLevels();
    cocos2d::CCArray* getSharedLevels();
    cocos2d::CCArray* getDiscoverLevels();

    bool isMyLevel(std::string_view key);
    bool isMyLevel(GJGameLevel* level);
    bool isSharedLevel(GJGameLevel* level);
    bool isDiscoverLevel(GJGameLevel* level);

    void setLevelValues(GJGameLevel* level, LevelEntry const& entry);
    void setLevelValues(LevelSetting* settings, GJGameLevel* level);

    std::optional<std::string> getLevelKey(GJGameLevel* level);
    LevelEntry* getLevelEntry(GJGameLevel* level);
    LevelEntry* getLevelEntry(std::string_view key);

    GJGameLevel* getShadowLevel(GJGameLevel* level);
    GJGameLevel* getReflectedLevel(GJGameLevel* level);
    bool isShadowLevel(GJGameLevel* level);

    void updateMyLevels(std::vector<LevelEntry>&& entries);
    void updateSharedLevels(std::vector<LevelEntry>&& entries);
    void updateDiscoverLevels(std::vector<LevelEntry>&& entries);

    void addLevelEntry(GJGameLevel* level, LevelEntry entry);
    void saveLevelEntry(LevelEntry const& entry);

    void createShadowLevel(GJGameLevel* level);
    void removeShadowLevel(GJGameLevel* level);
};

$on_mod(Loaded) {
    BrowserManager::get()->init();
}

void BrowserManager::Impl::init() {
    auto& container = Mod::get()->getSaveContainer();
    if (container.contains("level-entries")) {
        for (auto& [uniqueId, value] : container["level-entries"]) {
            if (GEODE_UNWRAP_IF_OK(entry, value.as<LevelEntry>())) {
                if (auto level = EditorIDs::getLevelByID(entry.uniqueId)) {
                    m_levelEntries[level] = entry;
                }
            }
        }
    }
}

GJGameLevel* BrowserManager::Impl::getShadowLevel(GJGameLevel* level) {
    auto it = m_shadowEntries.find(level);
    if (it != m_shadowEntries.end()) {
        return it->second;
    }
    return level;
}

GJGameLevel* BrowserManager::Impl::getReflectedLevel(GJGameLevel* level) {
    for (auto& [real, shadow] : m_shadowEntries) {
        if (shadow == level) {
            return real;
        }
    }
    return level;
}

bool BrowserManager::Impl::isShadowLevel(GJGameLevel* level) {
    for (auto& [real, shadow] : m_shadowEntries) {
        if (shadow == level) {
            return true;
        }
    }
    return false;
}

void BrowserManager::Impl::createShadowLevel(GJGameLevel* level) {
    auto shadowLevel = GJGameLevel::create();
    this->setLevelValues(shadowLevel, m_levelEntries[level]);
    m_shadowEntries[level] = shadowLevel;
    m_shadowMyLevels.push_back(shadowLevel);
}

void BrowserManager::Impl::removeShadowLevel(GJGameLevel* level) {
    m_shadowEntries.erase(level);
    std::erase(m_shadowMyLevels, level);
}

cocos2d::CCArray* BrowserManager::Impl::getShadowMyLevels() {
    auto arr = cocos2d::CCArray::create();
    for (auto& level : m_myLevels) {
        arr->addObject(level);
    }
    return arr;
}

cocos2d::CCArray* BrowserManager::Impl::getLocalLevels(int folder) {
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

cocos2d::CCArray* BrowserManager::Impl::getMyLevels() {
    auto arr = cocos2d::CCArray::create();
    for (auto& level : m_myLevels) {
        arr->addObject(level);
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

bool BrowserManager::Impl::isMyLevel(std::string_view key) {
    return std::find_if(m_myLevels.begin(), m_myLevels.end(), [&](auto& level) {
        if (auto entry = this->getLevelEntry(level)) {
            return entry->key == key;
        }
        return false;
    }) != m_myLevels.end();
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

LevelEntry* BrowserManager::Impl::getLevelEntry(GJGameLevel* level) {
    auto it = m_levelEntries.find(level);
    if (it == m_levelEntries.end()) {
        return nullptr;
    }
    return &it->second;
}

LevelEntry* BrowserManager::Impl::getLevelEntry(std::string_view key) {
    for (auto& [level, entry] : m_levelEntries) {
        if (entry.key == key) {
            return &entry;
        }
    }
    return nullptr;
}

void BrowserManager::Impl::setLevelValues(GJGameLevel* level, LevelEntry const& entry) {
    level->m_levelType = GJLevelType::Editor;
    level->m_levelName = entry.settings.title;
    std::vector<uint8_t> data(entry.settings.description.begin(), entry.settings.description.end());
    level->m_levelDesc = crypto::base64Encode(data, crypto::Base64Flags::UrlSafe);
}

void BrowserManager::Impl::setLevelValues(LevelSetting* settings, GJGameLevel* level) {
    settings->title = level->m_levelName;
    auto data = crypto::base64Decode(level->m_levelDesc, crypto::Base64Flags::UrlSafe);
    settings->description = std::string(data.begin(), data.end());
}

void BrowserManager::Impl::updateMyLevels(std::vector<LevelEntry>&& entries) {
    for (auto const& level : m_myLevels) {
        m_levelEntries.erase(level);
        m_shadowEntries.erase(level);
    }
    m_myLevels.clear();
    m_shadowMyLevels.clear();

    for (auto& entry : entries) {
        auto localLevels = CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels);

        if (auto it = std::find_if(localLevels.begin(), localLevels.end(), [&](auto& localLevel) {
            return EditorIDs::getID(localLevel) == entry.uniqueId;
        }); it != localLevels.end()) {
            log::debug("found local level with name {} level {}", entry.settings.title, *it);
            
            auto shadowLevel = GJGameLevel::create();
            this->setLevelValues(shadowLevel, entry);
            m_shadowMyLevels.push_back(shadowLevel);
            m_shadowEntries[*it] = shadowLevel;

            m_myLevels.push_back(*it);
            m_levelEntries[*it] = std::move(entry);
        }
        else {
            auto level = GJGameLevel::create();
            this->setLevelValues(level, entry);
            log::debug("adding level with name {} level {}", entry.settings.title, level);

            auto shadowLevel = GJGameLevel::create();
            this->setLevelValues(shadowLevel, entry);
            m_shadowMyLevels.push_back(shadowLevel);
            m_shadowEntries[level] = shadowLevel;

            m_myLevels.push_back(level);
            BrowserManager::get()->saveLevelEntry(entry);
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

void BrowserManager::Impl::addLevelEntry(GJGameLevel* level, LevelEntry entry) {
    entry.uniqueId = EditorIDs::getID(level);
    entry.hostAccountId = GJAccountManager::get()->m_accountID;
    this->setLevelValues(&entry.settings, level);

    log::debug("Adding level entry with level id {}, level {}", entry.uniqueId, level);

    m_levelEntries[level] = entry;
    Mod::get()->getSaveContainer()["level-entries"][fmt::format("{}", entry.uniqueId)] = entry;
}
void BrowserManager::Impl::saveLevelEntry(LevelEntry const& entry) {
    log::debug("Saving level entry with level id {}", entry.uniqueId);

    Mod::get()->getSaveContainer()["level-entries"][fmt::format("{}", entry.uniqueId)] = entry;
    if (auto level = EditorIDs::getLevelByID(entry.uniqueId)) {
        m_levelEntries[level] = entry;
    }
}

BrowserManager* BrowserManager::get() {
    static BrowserManager instance;
    return &instance;
}

BrowserManager::BrowserManager() : impl(std::make_unique<Impl>()) {}
BrowserManager::~BrowserManager() = default;

cocos2d::CCArray* BrowserManager::getShadowMyLevels() {
    return impl->getShadowMyLevels();
}

cocos2d::CCArray* BrowserManager::getLocalLevels(int folder) {
    return impl->getLocalLevels(folder);
}

cocos2d::CCArray* BrowserManager::getMyLevels() {
    return impl->getMyLevels();
}

cocos2d::CCArray* BrowserManager::getSharedLevels() {
    return impl->getSharedLevels();
}

cocos2d::CCArray* BrowserManager::getDiscoverLevels() {
    return impl->getDiscoverLevels();
}

bool BrowserManager::isMyLevel(std::string_view key) {
    return impl->isMyLevel(key);
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

LevelEntry* BrowserManager::getLevelEntry(GJGameLevel* level) {
    return impl->getLevelEntry(level);
}

LevelEntry* BrowserManager::getLevelEntry(std::string_view key) {
    return impl->getLevelEntry(key);
}

GJGameLevel* BrowserManager::getShadowLevel(GJGameLevel* level) {
    return impl->getShadowLevel(level);
}

GJGameLevel* BrowserManager::getReflectedLevel(GJGameLevel* level) {
    return impl->getReflectedLevel(level);
}

bool BrowserManager::isShadowLevel(GJGameLevel* level) {
    return impl->isShadowLevel(level);
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

void BrowserManager::init() {
    impl->init();
}

void BrowserManager::addLevelEntry(GJGameLevel* level, LevelEntry entry) {
    impl->addLevelEntry(level, entry);
}

void BrowserManager::saveLevelEntry(LevelEntry const& entry) {
    impl->saveLevelEntry(entry);
}

void BrowserManager::setLevelValues(GJGameLevel* level, LevelEntry const& entry) {
    impl->setLevelValues(level, entry);
}

void BrowserManager::createShadowLevel(GJGameLevel* level) {
    impl->createShadowLevel(level);
}

void BrowserManager::removeShadowLevel(GJGameLevel* level) {
    impl->removeShadowLevel(level);
}