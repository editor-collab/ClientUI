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
    
    struct LevelSetting {
        std::vector<std::string> viewers;
        std::vector<std::string> editors;
        std::vector<std::string> admins;

        std::string title;
        std::string description;

        DefaultSharingType defaultSharing;
        bool copyable;
        bool discoverable;
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
struct matjson::Serialize<tulip::editor::LevelSetting> {
    using LevelSetting = tulip::editor::LevelSetting;
    using DefaultSharingType = tulip::editor::DefaultSharingType;
    static matjson::Value to_json(LevelSetting const& entry) {
        auto value = matjson::Value();
        value.try_set("viewers", entry.viewers);
        value.try_set("editors", entry.editors);
        value.try_set("admins", entry.admins);
        value.try_set("title", entry.title);
        value.try_set("description", entry.description);
        value.try_set("default-sharing", entry.defaultSharing);
        value.try_set("copyable", entry.copyable);
        value.try_set("discoverable", entry.discoverable);
        return value;
    }
    static LevelSetting from_json(matjson::Value const& value) {
        LevelSetting entry;
        entry.viewers = value.try_get<std::vector<std::string>>("viewers").value_or(std::vector<std::string>{});
        entry.editors = value.try_get<std::vector<std::string>>("editors").value_or(std::vector<std::string>{});
        entry.admins = value.try_get<std::vector<std::string>>("admins").value_or(std::vector<std::string>{});
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