#pragma once

#include <cstdint>
#include <string>
#include <matjson.hpp>
#include "User.hpp"

namespace tulip::editor {
    struct ConnectedUserEntry {
        User user;
        uint32_t clientLevelId = 0;
        bool hanging = false;
        DefaultSharingType sharingType = DefaultSharingType::Restricted;
    };

    struct ConnectedUserList {
        std::vector<ConnectedUserEntry> users;

        bool contains(uint32_t clientLevelId) const {
            return std::any_of(users.begin(), users.end(), [clientLevelId](auto const& entry) {
                return entry.clientLevelId == clientLevelId;
            });
        }

        ConnectedUserEntry* find(uint32_t clientLevelId) {
            auto it = std::find_if(users.begin(), users.end(), [clientLevelId](auto const& entry) {
                return entry.clientLevelId == clientLevelId;
            });
            return it != users.end() ? &*it : nullptr;
        }

        void erase(uint32_t clientLevelId) {
            users.erase(std::remove_if(users.begin(), users.end(), [clientLevelId](auto const& entry) {
                return entry.clientLevelId == clientLevelId;
            }), users.end());
        }
    };
}

template <>
struct matjson::Serialize<tulip::editor::ConnectedUserEntry> {
    using ConnectedUserEntry = tulip::editor::ConnectedUserEntry;
    using User = tulip::editor::User;
    static matjson::Value toJson(ConnectedUserEntry const& entry) {
        auto value = matjson::Value();
        value["user"] = entry.user;
        value["client-level-id"] = entry.clientLevelId;
        value["hanging"] = entry.hanging;
        return value;
    }
    static geode::Result<ConnectedUserEntry> fromJson(matjson::Value const& value) {
        ConnectedUserEntry entry;
        entry.user = value["user"].as<User>().unwrapOrDefault();
        entry.clientLevelId = value["client-level-id"].asInt().unwrapOrDefault();
        entry.hanging = value["hanging"].asBool().unwrapOrDefault();
        return geode::Ok(entry);
    }
};

template <>
struct matjson::Serialize<tulip::editor::ConnectedUserList> {
    using ConnectedUserList = tulip::editor::ConnectedUserList;
    using ConnectedUserEntry = tulip::editor::ConnectedUserEntry;
    static matjson::Value toJson(ConnectedUserList const& entry) {
        auto value = matjson::Value();
        value["users"] = entry.users;
        return value;
    }
    static geode::Result<ConnectedUserList> fromJson(matjson::Value const& value) {
        ConnectedUserList entry;
        entry.users = value["users"].as<std::vector<ConnectedUserEntry>>().unwrapOrDefault();
        return geode::Ok(entry);
    }
};
