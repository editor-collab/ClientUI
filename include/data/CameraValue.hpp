#pragma once

#include <cstdint>
#include <string>
#include <matjson.hpp>

namespace tulip::editor {
    struct CameraValue {
        float x = 0;
        float y = 0;
        float zoom = 1;
    };
}

template <>
struct matjson::Serialize<tulip::editor::CameraValue> {
    static matjson::Value to_json(tulip::editor::CameraValue const& entry) {
        auto value = matjson::Value();
        value.try_set("x", entry.x);
        value.try_set("y", entry.y);
        value.try_set("zoom", entry.zoom);
        return value;
    }
    static tulip::editor::CameraValue from_json(matjson::Value const& value) {
        auto camera = tulip::editor::CameraValue();
        camera.x = value.try_get<float>("x").value_or(0);
        camera.y = value.try_get<float>("y").value_or(0);
        camera.zoom = value.try_get<float>("zoom").value_or(1);
        return camera;
    }
    static bool is_json(matjson::Value const& json) {
        return json.is_object();
    }
};