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
        uint32_t slotId = 0;
        uint32_t uniqueId = 0;
    };
}

template <>
struct matjson::Serialize<tulip::editor::LevelEntry> {
    using LevelEntry = tulip::editor::LevelEntry;
    using LevelSetting = tulip::editor::LevelSetting;
    static matjson::Value to_json(LevelEntry const& entry) {
        auto value = matjson::Value();
        value.try_set("settings", entry.settings);
        value.try_set("host-account-id", entry.hostAccountId);
        value.try_set("key", entry.key);
        value.try_set("user-count", entry.userCount);
        value.try_set("slot-id", entry.slotId);
        value.try_set("unique-id", entry.uniqueId);
        return value;
    }
    static LevelEntry from_json(matjson::Value const& value) {
        auto entry = LevelEntry();
        entry.settings = value.try_get<LevelSetting>("settings").value_or(LevelSetting());
        entry.hostAccountId = value.try_get<uint32_t>("host-account-id").value_or(0);
        entry.key = value.try_get<std::string>("key").value_or("");
        entry.userCount = value.try_get<uint32_t>("user-count").value_or(0);
        entry.slotId = value.try_get<uint32_t>("slot-id").value_or(0);
        entry.uniqueId = value.try_get<uint32_t>("unique-id").value_or(0);
        return entry;
    }
    static bool is_json(matjson::Value const& json) {
        return json.is_object();
    }
};