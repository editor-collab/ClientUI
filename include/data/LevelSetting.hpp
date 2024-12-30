#pragma once

#include <vector>
#include <string>
#include <matjson.hpp>
#include "User.hpp"
#include <Geode/Geode.hpp>

namespace tulip::editor {

    enum class DefaultSharingType {
        Restricted,
        Viewer,
        Editor,
        Admin,
    };

    struct SettingUserEntry {
        std::string name;
        DefaultSharingType role;
    };

    struct BannedUserEntry {
        User user;
        std::string reason;
    };
    
    struct LevelSetting {
        std::vector<SettingUserEntry> users;
        std::vector<BannedUserEntry> banned;

        std::string title;
        std::string description;

        DefaultSharingType defaultSharing = DefaultSharingType::Restricted;
        bool copyable = false;
        bool discoverable = false;

        static LevelSetting fromLevel(GJGameLevel* level) {
            LevelSetting entry;
            entry.title = level->m_levelName;
            entry.description = level->m_levelDesc;
            return entry;
        }

        DefaultSharingType getUserType(std::string_view name) const {
            for (auto const& user : users) {
                if (user.name == name) {
                    return user.role;
                }
            }
            return DefaultSharingType::Restricted;
        }

        bool hasUser(std::string_view name) const {
            return std::any_of(users.begin(), users.end(), [name](auto const& user) {
                return user.name == name;
            });
        }

        void removeUser(std::string_view name) {
            users.erase(std::remove_if(users.begin(), users.end(), [name](auto const& user) {
                return user.name == name;
            }), users.end());
        }

        void setUser(std::string_view name, DefaultSharingType type) {
            this->removeUser(name);
            users.push_back({std::string(name), type});
        }

        bool hasBanned(uint32_t accountId) const {
            return std::any_of(banned.begin(), banned.end(), [accountId](auto const& user) {
                return user.user.accountId == accountId;
            });
        }

        void setBanned(User user, std::string_view reason) {
            this->removeBanned(user.accountId);
            banned.push_back({user, std::string(reason)});
        }

        void removeBanned(uint32_t accountId) {
            banned.erase(std::remove_if(banned.begin(), banned.end(), [accountId](auto const& user) {
                return user.user.accountId == accountId;
            }), banned.end());
        }
    };
}

template <>
struct matjson::Serialize<tulip::editor::DefaultSharingType> {
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    static matjson::Value toJson(DefaultSharingType const& sharing) {
        switch (sharing) {
            case DefaultSharingType::Restricted: return "restricted";
            case DefaultSharingType::Viewer: return "viewer";
            case DefaultSharingType::Editor: return "editor";
            case DefaultSharingType::Admin: return "admin";
        }
    }
    static geode::Result<DefaultSharingType> fromJson(matjson::Value const& value) {
        auto str = value.asString().unwrapOrDefault();
        if (str == "restricted") return geode::Ok(DefaultSharingType::Restricted);
        if (str == "viewer") return geode::Ok(DefaultSharingType::Viewer);
        if (str == "editor") return geode::Ok(DefaultSharingType::Editor);
        if (str == "admin") return geode::Ok(DefaultSharingType::Admin);
        return geode::Ok(DefaultSharingType::Restricted);
    }
};

template <>
struct matjson::Serialize<tulip::editor::SettingUserEntry> {
    using SettingUserEntry = tulip::editor::SettingUserEntry;
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    static matjson::Value toJson(SettingUserEntry const& entry) {
        auto value = matjson::Value();
        value["name"] = entry.name;
        value["role"] = entry.role;
        return value;
    }
    static geode::Result<SettingUserEntry> fromJson(matjson::Value const& value) {
        SettingUserEntry entry;
        entry.name = value["name"].asString().unwrapOrDefault();
        entry.role = value["role"].as<DefaultSharingType>().unwrapOr(DefaultSharingType::Viewer);
        return geode::Ok(entry);
    }
};

template <>
struct matjson::Serialize<tulip::editor::BannedUserEntry> {
    using BannedUserEntry = tulip::editor::BannedUserEntry;
    using User = tulip::editor::User;
    static matjson::Value toJson(BannedUserEntry const& entry) {
        auto value = matjson::Value();
        value["user"] = entry.user;
        value["reason"] = entry.reason;
        return value;
    }
    static geode::Result<BannedUserEntry> fromJson(matjson::Value const& value) {
        BannedUserEntry entry;
        entry.user = value["user"].as<User>().unwrapOrDefault();
        entry.reason = value["reason"].asString().unwrapOrDefault();
        return geode::Ok(entry);
    }
};

template <>
struct matjson::Serialize<tulip::editor::LevelSetting> {
    using LevelSetting = tulip::editor::LevelSetting;
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    using SettingUserEntry = tulip::editor::SettingUserEntry;
    using BannedUserEntry = tulip::editor::BannedUserEntry;
    static matjson::Value toJson(LevelSetting const& entry) {
        auto value = matjson::Value();
        value["users"] = entry.users;
        value["banned"] = entry.banned;
        value["title"] = entry.title;
        value["description"] = entry.description;
        value["default-sharing"] = entry.defaultSharing;
        value["copyable"] = entry.copyable;
        value["discoverable"] = entry.discoverable;
        return value;
    }
    static geode::Result<LevelSetting> fromJson(matjson::Value const& value) {
        LevelSetting entry;
        entry.users = value["users"].as<std::vector<SettingUserEntry>>().unwrapOrDefault();
        entry.banned = value["banned"].as<std::vector<BannedUserEntry>>().unwrapOrDefault();
        entry.title = value["title"].asString().unwrapOrDefault();
        entry.description = value["description"].asString().unwrapOrDefault();
        entry.defaultSharing = value["default-sharing"].as<DefaultSharingType>().unwrapOr(DefaultSharingType::Restricted);
        entry.copyable = value["copyable"].asBool().unwrapOr(false);
        entry.discoverable = value["discoverable"].asBool().unwrapOr(false);
        return geode::Ok(entry);
    }
};