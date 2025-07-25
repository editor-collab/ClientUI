#include <managers/BrowserManager.hpp>
#include <managers/LocalManager.hpp>
#include <data/LevelEntry.hpp>
#include <utils/CryptoHelper.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class BrowserManager::Impl {
public:
    class Reflected;
    class Shadow {
        Reflected* reflected = nullptr;
    public:
        Shadow(Reflected* reflected) : reflected(reflected) {}

        std::unordered_map<ShadowLevel*, LevelEntry> m_levelEntries;

        std::vector<Ref<ShadowLevel>> m_myLevels;
        std::vector<Ref<ShadowLevel>> m_sharedLevels;
        std::vector<Ref<ShadowLevel>> m_discoverLevels;

        CCArray* getMyLevels();
        CCArray* getSharedLevels();
        CCArray* getDiscoverLevels();

        bool isMyLevel(ShadowLevel* level);
        bool isSharedLevel(ShadowLevel* level);
        bool isDiscoverLevel(ShadowLevel* level);
        bool isOnlineLevel(ShadowLevel* level);

        void clearMyLevels();
        void clearSharedLevels();
        void clearDiscoverLevels();

        void addMyLevel(ShadowLevel* level, LevelEntry&& entry);
        void addSharedLevel(ShadowLevel* level, LevelEntry&& entry);
        void addDiscoverLevel(ShadowLevel* level, LevelEntry&& entry);

        void setLevelValues(ShadowLevel* level, LevelEntry const& entry);
        void setLevelValues(LevelSetting* settings, ShadowLevel* level);

        void addLevelEntry(ShadowLevel* level, LevelEntry&& entry);
        void updateLevelEntry(ShadowLevel* level);
        LevelEntry* getLevelEntry(ShadowLevel* level);
        bool hasLevelEntry(ShadowLevel* level);

        bool hasOnlineEntry(ShadowLevel* level);
        LevelEntry* getOnlineEntry(ShadowLevel* level);

        void updateMyLevels(std::vector<LevelEntry>&& entries);
        void updateSharedLevels(std::vector<LevelEntry>&& entries);
        void updateDiscoverLevels(std::vector<LevelEntry>&& entries);
    };

    class Reflected {
        Shadow* shadow = nullptr;
    public:
        Reflected(Shadow* shadow) : shadow(shadow) {}

        std::unordered_map<ReflectedLevel*, Ref<ShadowLevel>> m_shadowMapping;

        std::vector<Ref<ReflectedLevel>> m_myLevels;

        ShadowLevel* getShadowLevel(ReflectedLevel* level);
        ReflectedLevel* getReflectedLevel(ShadowLevel* level);

        CCArray* getMyLevels();

        void clearMyLevels();

        void addMyLevel(ReflectedLevel* level);

        bool isMyLevel(ReflectedLevel* level);

        ShadowLevel* createShadowLevel(ReflectedLevel* level);
        ShadowLevel* removeShadowLevel(ReflectedLevel* level);

        ReflectedLevel* createReflectedLevel(ShadowLevel* level);
        ReflectedLevel* removeReflectedLevel(ShadowLevel* level);

        void saveLevel(ReflectedLevel* level, bool insert);

        std::variant<ReflectedLevel*, ShadowLevel*, GJGameLevel*> convertLevel(GJGameLevel* level);
    };

    Shadow shadow = &this->reflected;
    Reflected reflected = &this->shadow;

    void init();

    cocos2d::CCArray* getLocalLevels(int folder);
    cocos2d::CCArray* getMyLevels();
    cocos2d::CCArray* getSharedLevels();
    cocos2d::CCArray* getDiscoverLevels();

    bool isMyLevel(GJGameLevel* level);
    bool isSharedLevel(GJGameLevel* level);
    bool isDiscoverLevel(GJGameLevel* level);
    bool isOnlineLevel(GJGameLevel* level);

    bool hasLevelEntry(GJGameLevel* level);
    void addLevelEntry(GJGameLevel* level, LevelEntry&& entry);
    void updateLevelEntry(GJGameLevel* level);
    LevelEntry* getLevelEntry(GJGameLevel* level);

    bool hasOnlineEntry(GJGameLevel* level);
    LevelEntry* getOnlineEntry(GJGameLevel* level);

    void updateMyLevels(std::vector<LevelEntry>&& entries);
    void updateSharedLevels(std::vector<LevelEntry>&& entries);
    void updateDiscoverLevels(std::vector<LevelEntry>&& entries);

    void setLevelValues(GJGameLevel* level, LevelEntry const& entry);

    void saveLevel(GJGameLevel* level, bool insert);

    void replaceWithShadowLevel(GJGameLevel*& level);
};

$on_mod(Loaded) {
    BrowserManager::get()->init();
}

void BrowserManager::Impl::init() {
    // auto& container = Mod::get()->getSaveContainer();
    // auto accountId = GJAccountManager::get()->m_accountID;
    // if (container.contains("level-entries")) {
    //     for (auto& [uniqueId, value] : container["level-entries"]) {
    //         if (GEODE_UNWRAP_IF_OK(entry, value.as<LevelEntry>())) {
    //             if (auto reflectedLevel = static_cast<ReflectedLevel*>(EditorIDs::getLevelByID(entry.uniqueId))) {
    //                 auto shadowLevel = reflected.createShadowLevel(reflectedLevel);
                    
    //                 if (accountId == entry.hostAccountId) {
    //                     shadow.addMyLevel(shadowLevel, std::move(entry));
    //                 }
    //                 else {
    //                     shadow.addSharedLevel(shadowLevel, std::move(entry));
    //                 }
    //             }
    //             else {
    //                 auto shadowLevel = ShadowLevel::create();
                    
    //                 if (accountId == entry.hostAccountId) {
    //                     shadow.addMyLevel(shadowLevel, std::move(entry));
    //                 }
    //                 else {
    //                     shadow.addSharedLevel(shadowLevel, std::move(entry));
    //                 }
    //             }
    //         }
    //     }
    // }
}

CCArray* BrowserManager::Impl::Shadow::getMyLevels() {
    auto arr = CCArray::create();
    for (auto& level : m_myLevels) {
        arr->addObject(level);
    }
    return arr;
}

CCArray* BrowserManager::Impl::Shadow::getSharedLevels() {
    auto arr = CCArray::create();
    for (auto& level : m_sharedLevels) {
        arr->addObject(level);
    }
    return arr;
}

CCArray* BrowserManager::Impl::Shadow::getDiscoverLevels() {
    auto arr = CCArray::create();
    for (auto& level : m_discoverLevels) {
        arr->addObject(level);
    }
    return arr;
}

bool BrowserManager::Impl::Shadow::isMyLevel(ShadowLevel* level) {
    return std::find(m_myLevels.begin(), m_myLevels.end(), level) != m_myLevels.end();
}
bool BrowserManager::Impl::Shadow::isSharedLevel(ShadowLevel* level) {
    return std::find(m_sharedLevels.begin(), m_sharedLevels.end(), level) != m_sharedLevels.end();
}
bool BrowserManager::Impl::Shadow::isDiscoverLevel(ShadowLevel* level) {
    return std::find(m_discoverLevels.begin(), m_discoverLevels.end(), level) != m_discoverLevels.end();
}
bool BrowserManager::Impl::Shadow::isOnlineLevel(ShadowLevel* level) {
    return this->isMyLevel(level) || this->isSharedLevel(level) || this->isDiscoverLevel(level);
}

void BrowserManager::Impl::Shadow::clearMyLevels() {
    std::erase_if(m_levelEntries, [&](auto& pair) {
        return std::find(m_myLevels.begin(), m_myLevels.end(), pair.first) != m_myLevels.end();
    });
    m_myLevels.clear();
}
void BrowserManager::Impl::Shadow::clearSharedLevels() {
    std::erase_if(m_levelEntries, [&](auto& pair) {
        return std::find(m_sharedLevels.begin(), m_sharedLevels.end(), pair.first) != m_sharedLevels.end();
    });
    m_sharedLevels.clear();
}

void BrowserManager::Impl::Shadow::clearDiscoverLevels() {
    std::erase_if(m_levelEntries, [&](auto& pair) {
        return std::find(m_discoverLevels.begin(), m_discoverLevels.end(), pair.first) != m_discoverLevels.end();
    });
    m_discoverLevels.clear();
}

void BrowserManager::Impl::Shadow::addMyLevel(ShadowLevel* level, LevelEntry&& entry) {
    m_myLevels.push_back(level);
    this->addLevelEntry(level, std::move(entry));
}

void BrowserManager::Impl::Shadow::addSharedLevel(ShadowLevel* level, LevelEntry&& entry) {
    m_sharedLevels.push_back(level);
    this->addLevelEntry(level, std::move(entry));
}

void BrowserManager::Impl::Shadow::addDiscoverLevel(ShadowLevel* level, LevelEntry&& entry) {
    m_discoverLevels.push_back(level);
    this->addLevelEntry(level, std::move(entry));
}

void BrowserManager::Impl::Shadow::setLevelValues(ShadowLevel* level, LevelEntry const& entry) {
    level->m_levelName = entry.settings.title;
    std::vector<uint8_t> data(entry.settings.description.begin(), entry.settings.description.end());
    level->m_levelDesc = crypto::base64Encode(data, crypto::Base64Flags::UrlSafe);
}
void BrowserManager::Impl::Shadow::setLevelValues(LevelSetting* settings, ShadowLevel* level) {
    settings->title = level->m_levelName;
    auto data = crypto::base64Decode(level->m_levelDesc, crypto::Base64Flags::UrlSafe);
    settings->description = std::string(data.begin(), data.end());
}

void BrowserManager::Impl::Shadow::addLevelEntry(ShadowLevel* level, LevelEntry&& entry) {
    this->setLevelValues(&entry.settings, level);

    log::debug("Adding level entry with level id {}, level {}", entry.uniqueId, level);

    m_levelEntries[level] = std::move(entry);
    Mod::get()->getSaveContainer()["level-entries"][fmt::format("{}", entry.uniqueId)] = entry;
}

void BrowserManager::Impl::Shadow::updateLevelEntry(ShadowLevel* level) {
    auto entry = this->getLevelEntry(level);
    this->setLevelValues(&entry->settings, level);
    Mod::get()->getSaveContainer()["level-entries"][fmt::format("{}", entry->uniqueId)] = *entry;
}

LevelEntry* BrowserManager::Impl::Shadow::getLevelEntry(ShadowLevel* level) {
    if (auto it = m_levelEntries.find(level); it != m_levelEntries.end()) {
        return &it->second;
    }
    return nullptr;
}

bool BrowserManager::Impl::Shadow::hasLevelEntry(ShadowLevel* level) {
    return m_levelEntries.find(level) != m_levelEntries.end();
}

bool BrowserManager::Impl::Shadow::hasOnlineEntry(ShadowLevel* level) {
    return this->isMyLevel(level) || this->isSharedLevel(level) || this->isDiscoverLevel(level);
}

LevelEntry* BrowserManager::Impl::Shadow::getOnlineEntry(ShadowLevel* level) {
    if (this->hasOnlineEntry(level)) {
        return this->getLevelEntry(level);
    }
    return nullptr;
}

void BrowserManager::Impl::Shadow::updateMyLevels(std::vector<LevelEntry>&& entries) {
    this->clearMyLevels();

    for (auto& entry : entries) {
        auto localLevels = CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels);

        if (auto it = std::find_if(localLevels.begin(), localLevels.end(), [&](auto& localLevel) {
            return EditorIDs::getID(localLevel) == entry.uniqueId;
        }); it != localLevels.end()) {
            log::debug("found reflected level with name {}", entry.settings.title);
            auto reflectedLevel = static_cast<ReflectedLevel*>(*it);

            if (this->reflected->isMyLevel(reflectedLevel)) {
                auto shadowLevel = this->reflected->getShadowLevel(reflectedLevel);
                log::debug("shadow level {} already exists for reflected level {}", shadowLevel, reflectedLevel);
                this->addLevelEntry(shadowLevel, std::move(entry));
            }
            else {
                auto shadowLevel = this->reflected->createShadowLevel(reflectedLevel);
                log::debug("creating a shadow level {} for existing reflected level {}", shadowLevel, reflectedLevel);

                this->addMyLevel(shadowLevel, std::move(entry));
            }
        }
        else {
            auto shadowLevel = ShadowLevel::create();
            log::debug("creating new shadow level {} for level name {}", shadowLevel, entry.settings.title);
            
            this->addMyLevel(shadowLevel, std::move(entry));
        }
    }
}

void BrowserManager::Impl::Shadow::updateSharedLevels(std::vector<LevelEntry>&& entries) {
    this->clearSharedLevels();

    for (auto& entry : entries) {
        auto shadowLevel = ShadowLevel::create();
        this->addSharedLevel(shadowLevel, std::move(entry));
    }
}

void BrowserManager::Impl::Shadow::updateDiscoverLevels(std::vector<LevelEntry>&& entries) {
    this->clearDiscoverLevels();

    for (auto& entry : entries) {
        auto shadowLevel = ShadowLevel::create();
        this->addDiscoverLevel(shadowLevel, std::move(entry));
    }
}

ShadowLevel* BrowserManager::Impl::Reflected::getShadowLevel(ReflectedLevel* level) {
    if (auto it = m_shadowMapping.find(level); it != m_shadowMapping.end()) {
        return it->second.data();
    }
    return nullptr;
}

ReflectedLevel* BrowserManager::Impl::Reflected::getReflectedLevel(ShadowLevel* level) {
    for (auto& pair : m_shadowMapping) {
        if (pair.second == level) {
            return pair.first;
        }
    }
    return nullptr;
}

CCArray* BrowserManager::Impl::Reflected::getMyLevels() {
    CCArray* array = CCArray::create();
    for (auto& level : m_myLevels) {
        array->addObject(level);
    }
    return array;
}

void BrowserManager::Impl::Reflected::clearMyLevels() {
    std::erase_if(m_shadowMapping, [&](auto& pair) {
        return std::find(m_myLevels.begin(), m_myLevels.end(), pair.first) != m_myLevels.end();
    });
    m_myLevels.clear();
}

void BrowserManager::Impl::Reflected::addMyLevel(ReflectedLevel* level) {
    m_myLevels.push_back(level);
}

bool BrowserManager::Impl::Reflected::isMyLevel(ReflectedLevel* level) {
    return std::find(m_myLevels.begin(), m_myLevels.end(), level) != m_myLevels.end();
}

ShadowLevel* BrowserManager::Impl::Reflected::createShadowLevel(ReflectedLevel* level) {
    auto shadowLevel = ShadowLevel::create();
    m_shadowMapping[level] = shadowLevel;
    shadowLevel->m_levelName = level->m_levelName;
    shadowLevel->m_levelDesc = level->m_levelDesc;
    shadowLevel->m_songID = level->m_songID;
    shadowLevel->m_songIDs = level->m_songIDs;
    shadowLevel->m_levelLength = level->m_levelLength;
    return shadowLevel;
}

ShadowLevel* BrowserManager::Impl::Reflected::removeShadowLevel(ReflectedLevel* level) {
    if (auto it = m_shadowMapping.find(level); it != m_shadowMapping.end()) {
        auto shadowLevel = it->second.data();
        m_shadowMapping.erase(it);
        return shadowLevel;
    }
    return nullptr;
}

ReflectedLevel* BrowserManager::Impl::Reflected::createReflectedLevel(ShadowLevel* level) {
    auto reflectedLevel = ReflectedLevel::create();
    m_shadowMapping[reflectedLevel] = level;
    reflectedLevel->m_levelName = level->m_levelName;
    reflectedLevel->m_levelDesc = level->m_levelDesc;
    reflectedLevel->m_songID = level->m_songID;
    reflectedLevel->m_songIDs = level->m_songIDs;
    reflectedLevel->m_levelLength = level->m_levelLength;
    return reflectedLevel;
}

ReflectedLevel* BrowserManager::Impl::Reflected::removeReflectedLevel(ShadowLevel* level) {
    for (auto it = m_shadowMapping.begin(); it != m_shadowMapping.end(); ++it) {
        if (it->second == level) {
            auto reflectedLevel = it->first;
            m_shadowMapping.erase(it);
            return reflectedLevel;
        }
    }
    return nullptr;
}

void BrowserManager::Impl::Reflected::saveLevel(ReflectedLevel* level, bool insert) {
    auto localLevels = CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels);
    // for (auto i = 0; i < 5;) {
    //     if (localLevels[i]->m_levelType != GJLevelType::Editor) {
    //         LocalLevelManager::get()->m_localLevels->removeObjectAtIndex(i);
    //     }
    //     else i++;
    // }
    if (insert && std::find(localLevels.begin(), localLevels.end(), level) == localLevels.end()) {
        log::debug("Level not found in local levels, adding level {}", level);
        LocalLevelManager::get()->m_localLevels->insertObject(level, 0);
    }
}

std::variant<ReflectedLevel*, ShadowLevel*, GJGameLevel*> BrowserManager::Impl::Reflected::convertLevel(GJGameLevel* level) {
    auto possibleShadow = static_cast<ShadowLevel*>(level);
    if (shadow->hasLevelEntry(possibleShadow)) {
        return possibleShadow;
    }
    auto possibleReflected = static_cast<ReflectedLevel*>(level);
    if (auto shadowLevel = this->getShadowLevel(possibleReflected)) {
        return possibleReflected;
    }
    return level;
}

cocos2d::CCArray* BrowserManager::Impl::getLocalLevels(int folder) {
    auto arr = shadow.getMyLevels();
    for (auto level : CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels)) {
        if (level->m_levelFolder == folder) {
            auto variant = reflected.convertLevel(level);
            std::visit(makeVisitor {
                [&](ShadowLevel* shadowLevel) {
                    if (!shadow.isMyLevel(shadowLevel)) arr->addObject(level);
                },
                [&](ReflectedLevel* reflectedLevel) {
                    if (!reflected.isMyLevel(reflectedLevel)) arr->addObject(level);
                },
                [&](GJGameLevel* gjLevel) {
                    arr->addObject(gjLevel);
                }
            }, variant);
        }
    }
    return arr;
}
cocos2d::CCArray* BrowserManager::Impl::getMyLevels() {
    return shadow.getMyLevels();
}
cocos2d::CCArray* BrowserManager::Impl::getSharedLevels() {
    return shadow.getSharedLevels();
}
cocos2d::CCArray* BrowserManager::Impl::getDiscoverLevels() {
    return shadow.getDiscoverLevels();
}

bool BrowserManager::Impl::isMyLevel(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.isMyLevel(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return false; },
        [&](GJGameLevel* gjLevel) { return false; }
    }, variant);
}

bool BrowserManager::Impl::isSharedLevel(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.isSharedLevel(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return false; },
        [&](GJGameLevel* gjLevel) { return false; }
    }, variant);
}

bool BrowserManager::Impl::isDiscoverLevel(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.isDiscoverLevel(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return false; },
        [&](GJGameLevel* gjLevel) { return false; }
    }, variant);
}

bool BrowserManager::Impl::isOnlineLevel(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.isOnlineLevel(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return false; },
        [&](GJGameLevel* gjLevel) { return false; }
    }, variant);
}

bool BrowserManager::Impl::hasLevelEntry(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.hasLevelEntry(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return shadow.hasLevelEntry(reflected.getShadowLevel(reflectedLevel)); },
        [&](GJGameLevel* gjLevel) { return false; }
    }, variant);
}
void BrowserManager::Impl::addLevelEntry(GJGameLevel* level, LevelEntry&& entry) {
    auto variant = reflected.convertLevel(level);
    std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { shadow.addLevelEntry(shadowLevel, std::move(entry)); },
        [&](ReflectedLevel* reflectedLevel) {
            auto shadowLevel = reflected.getShadowLevel(reflectedLevel);
            if (!shadowLevel) {
                shadowLevel = reflected.createShadowLevel(reflectedLevel);
            }
            shadow.addLevelEntry(shadowLevel, std::move(entry));
        },
        [&](GJGameLevel* gjLevel) {
            auto reflectedLevel = static_cast<ReflectedLevel*>(gjLevel);
            auto shadowLevel = reflected.createShadowLevel(reflectedLevel);
            shadow.addLevelEntry(shadowLevel, std::move(entry));
        }
    }, variant);
}
void BrowserManager::Impl::updateLevelEntry(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { shadow.updateLevelEntry(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) {
            auto shadowLevel = reflected.getShadowLevel(reflectedLevel);
            if (!shadowLevel) {
                shadowLevel = reflected.createShadowLevel(reflectedLevel);
            }
            shadow.updateLevelEntry(shadowLevel);
        },
        [&](GJGameLevel* gjLevel) { }
    }, variant);
}

LevelEntry* BrowserManager::Impl::getLevelEntry(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.getLevelEntry(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return shadow.getLevelEntry(reflected.getShadowLevel(reflectedLevel)); },
        [&](GJGameLevel* gjLevel) -> LevelEntry* { return nullptr; }
    }, variant);
}

bool BrowserManager::Impl::hasOnlineEntry(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.hasOnlineEntry(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return shadow.hasOnlineEntry(reflected.getShadowLevel(reflectedLevel)); },
        [&](GJGameLevel* gjLevel) { return false; }
    }, variant);
}

LevelEntry* BrowserManager::Impl::getOnlineEntry(GJGameLevel* level) {
    auto variant = reflected.convertLevel(level);
    return std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { return shadow.getOnlineEntry(shadowLevel); },
        [&](ReflectedLevel* reflectedLevel) { return shadow.getOnlineEntry(reflected.getShadowLevel(reflectedLevel)); },
        [&](GJGameLevel* gjLevel) -> LevelEntry* { return nullptr; }
    }, variant);
}

void BrowserManager::Impl::updateMyLevels(std::vector<LevelEntry>&& entries) {
    shadow.updateMyLevels(std::move(entries));
}

void BrowserManager::Impl::updateSharedLevels(std::vector<LevelEntry>&& entries) {
    shadow.updateSharedLevels(std::move(entries));
}
void BrowserManager::Impl::updateDiscoverLevels(std::vector<LevelEntry>&& entries) {
    shadow.updateDiscoverLevels(std::move(entries));
}

void BrowserManager::Impl::setLevelValues(GJGameLevel* level, LevelEntry const& entry) {
    auto variant = reflected.convertLevel(level);
    std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { shadow.setLevelValues(shadowLevel, entry); },
        [&](ReflectedLevel* reflectedLevel) {
            auto shadowLevel = reflected.getShadowLevel(reflectedLevel);
            if (!shadowLevel) {
                shadowLevel = reflected.createShadowLevel(reflectedLevel);
            }
            shadow.setLevelValues(shadowLevel, entry);
        },
        [&](GJGameLevel* gjLevel) { }
    }, variant);
}

void BrowserManager::Impl::saveLevel(GJGameLevel* level, bool insert) {
    auto variant = reflected.convertLevel(level);
    std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { 
            auto reflectedLevel = reflected.getReflectedLevel(shadowLevel);
            if (!reflectedLevel) {
                reflectedLevel = reflected.createReflectedLevel(shadowLevel);
                reflectedLevel->m_levelString = shadowLevel->m_levelString;
                reflected.saveLevel(reflectedLevel, insert);
            }
            else {
                reflectedLevel->m_levelString = shadowLevel->m_levelString;
                reflected.saveLevel(reflectedLevel, insert);
            }
        },
        [&](ReflectedLevel* reflectedLevel) {
            reflected.saveLevel(reflectedLevel, insert);
        },
        [&](GJGameLevel* gjLevel) { }
    }, variant);
}

void BrowserManager::Impl::replaceWithShadowLevel(GJGameLevel*& level) {
    auto variant = reflected.convertLevel(level);
    std::visit(makeVisitor {
        [&](ShadowLevel* shadowLevel) { },
        [&](ReflectedLevel* reflectedLevel) {
            auto shadowLevel = reflected.getShadowLevel(reflectedLevel);
            level = shadowLevel;
        },
        [&](GJGameLevel* gjLevel) {
            auto reflectedLevel = static_cast<ReflectedLevel*>(gjLevel);
            auto shadowLevel = reflected.createShadowLevel(reflectedLevel);
            level = shadowLevel;
        }
    }, variant);
}

BrowserManager* BrowserManager::get() {
    static BrowserManager instance;
    return &instance;
}

BrowserManager::BrowserManager() : impl(std::make_unique<Impl>()) {}
BrowserManager::~BrowserManager() = default;

void BrowserManager::init() {
    impl->init();
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

bool BrowserManager::isMyLevel(GJGameLevel* level) {
    return impl->isMyLevel(level);
}
bool BrowserManager::isSharedLevel(GJGameLevel* level) {
    return impl->isSharedLevel(level);
}
bool BrowserManager::isDiscoverLevel(GJGameLevel* level) {
    return impl->isDiscoverLevel(level);
}
bool BrowserManager::isOnlineLevel(GJGameLevel* level) {
    return impl->isOnlineLevel(level);
}

bool BrowserManager::hasLevelEntry(GJGameLevel* level) {
    return impl->hasLevelEntry(level);
}
void BrowserManager::addLevelEntry(GJGameLevel* level, LevelEntry&& entry) {
    impl->addLevelEntry(level, std::move(entry));
}
void BrowserManager::updateLevelEntry(GJGameLevel* level) {
    impl->updateLevelEntry(level);
}
LevelEntry* BrowserManager::getLevelEntry(GJGameLevel* level) {
    return impl->getLevelEntry(level);
}

bool BrowserManager::hasOnlineEntry(GJGameLevel* level) {
    return impl->hasOnlineEntry(level);
}
LevelEntry* BrowserManager::getOnlineEntry(GJGameLevel* level) {
    return impl->getOnlineEntry(level);
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

void BrowserManager::setLevelValues(GJGameLevel* level, LevelEntry const& entry) {
    impl->setLevelValues(level, entry);
}
void BrowserManager::saveLevel(GJGameLevel* level, bool insert) {
    impl->saveLevel(level, insert);
}

void BrowserManager::replaceWithShadowLevel(GJGameLevel*& level) {
    impl->replaceWithShadowLevel(level);
}