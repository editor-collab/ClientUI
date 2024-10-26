#pragma once

#include <vector>
#include <string>
#include <matjson.hpp>
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
    
    struct LevelSetting {
        std::vector<SettingUserEntry> users;

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
    };
}

template <>
struct matjson::Serialize<tulip::editor::DefaultSharingType> {
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    static matjson::Value to_json(DefaultSharingType const& sharing) {
        switch (sharing) {
            case DefaultSharingType::Restricted: return "restricted";
            case DefaultSharingType::Viewer: return "viewer";
            case DefaultSharingType::Editor: return "editor";
            case DefaultSharingType::Admin: return "admin";
        }
    }
    static DefaultSharingType from_json(matjson::Value const& value) {
        if (value == "restricted") return DefaultSharingType::Restricted;
        if (value == "viewer") return DefaultSharingType::Viewer;
        if (value == "editor") return DefaultSharingType::Editor;
        if (value == "admin") return DefaultSharingType::Admin;
        return DefaultSharingType::Restricted;
    }
    static bool is_json(matjson::Value const& json) {
        return json.is_string();
    }
};

template <>
struct matjson::Serialize<tulip::editor::SettingUserEntry> {
    using SettingUserEntry = tulip::editor::SettingUserEntry;
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    static matjson::Value to_json(SettingUserEntry const& entry) {
        auto value = matjson::Value();
        value.try_set("name", entry.name);
        value.try_set("role", entry.role);
        return value;
    }
    static SettingUserEntry from_json(matjson::Value const& value) {
        SettingUserEntry entry;
        entry.name = value.try_get<std::string>("name").value_or("");
        entry.role = value.try_get<DefaultSharingType>("role").value_or(DefaultSharingType::Viewer);
        return entry;
    }
    static bool is_json(matjson::Value const& json) {
        return json.is_object();
    }
};

template <>
struct matjson::Serialize<tulip::editor::LevelSetting> {
    using LevelSetting = tulip::editor::LevelSetting;
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    using SettingUserEntry = tulip::editor::SettingUserEntry;
    static matjson::Value to_json(LevelSetting const& entry) {
        auto value = matjson::Value();
        value.try_set("users", entry.users);
        value.try_set("title", entry.title);
        value.try_set("description", entry.description);
        value.try_set("default-sharing", entry.defaultSharing);
        value.try_set("copyable", entry.copyable);
        value.try_set("discoverable", entry.discoverable);
        return value;
    }
    static LevelSetting from_json(matjson::Value const& value) {
        LevelSetting entry;
        entry.users = value.try_get<std::vector<SettingUserEntry>>("users").value_or(std::vector<SettingUserEntry>{});
        entry.title = value.try_get<std::string>("title").value_or("");
        entry.description = value.try_get<std::string>("description").value_or("");
        entry.defaultSharing = value.try_get<DefaultSharingType>("default-sharing").value_or(DefaultSharingType::Restricted);
        entry.copyable = value.try_get<bool>("copyable").value_or(false);
        entry.discoverable = value.try_get<bool>("discoverable").value_or(false);
        return entry;
    }
    static bool is_json(matjson::Value const& json) {
        return json.is_object();
    }
};