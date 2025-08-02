#pragma once

#include <cstdint>
#include <string>
#include "LevelSetting.hpp"
#include <matjson.hpp>

namespace tulip::editor {
    struct LevelEntry {
        LevelSetting settings;
        uint32_t hostAccountId = 0;
        std::string key;
        uint32_t userCount = 0;

        bool isShared() const {
            return !key.empty();
        }
    };
}

template <>
struct matjson::Serialize<tulip::editor::LevelEntry> {
    using LevelEntry = tulip::editor::LevelEntry;
    using LevelSetting = tulip::editor::LevelSetting;
    static matjson::Value toJson(LevelEntry const& entry) {
        auto value = matjson::Value();
        value["settings"] = entry.settings;
        value["host-account-id"] = entry.hostAccountId;
        value["key"] = entry.key;
        value["user-count"] = entry.userCount;
        return value;
    }
    static geode::Result<LevelEntry> fromJson(matjson::Value const& value) {
        auto entry = LevelEntry();
        entry.settings = value["settings"].as<LevelSetting>().unwrapOrDefault();
        entry.hostAccountId = value["host-account-id"].asInt().unwrapOrDefault();
        entry.key = value["key"].asString().unwrapOrDefault();
        entry.userCount = value["user-count"].asInt().unwrapOrDefault();
        return geode::Ok(entry);
    }
};