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
    static matjson::Value toJson(tulip::editor::CameraValue const& entry) {
        auto value = matjson::Value();
        value["x"] = entry.x;
        value["y"] = entry.y;
        value["zoom"] = entry.zoom;
        return value;
    }
    static geode::Result<tulip::editor::CameraValue> fromJson(matjson::Value const& value) {
        auto camera = tulip::editor::CameraValue();
        camera.x = value["x"].asDouble().unwrapOrDefault();
        camera.y = value["y"].asDouble().unwrapOrDefault();
        camera.zoom = value["zoom"].asDouble().unwrapOr(1.0);
        return geode::Ok(camera);
    }
};