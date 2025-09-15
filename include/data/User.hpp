#pragma once

#include <vector>
#include <string>
#include <matjson.hpp>
#include <Geode/Geode.hpp>

namespace tulip::editor {
    struct User {
        uint32_t accountId;
        uint32_t userId;
        std::string accountName;
    };
}

template <>
struct matjson::Serialize<tulip::editor::User> {
    using User = tulip::editor::User;
    static matjson::Value toJson(User const& entry) {
        auto value = matjson::Value();
        value["account-id"] = entry.accountId;
        value["user-id"] = entry.userId;
        value["account-name"] = entry.accountName;
        return value;
    }
    static geode::Result<User> fromJson(matjson::Value const& value) {
        auto entry = User();
        entry.accountId = value["account-id"].asInt().unwrapOrDefault();
        entry.userId = value["user-id"].asInt().unwrapOrDefault();
        entry.accountName = value["account-name"].asString().unwrapOrDefault();
        return geode::Ok(entry);
    }
};
