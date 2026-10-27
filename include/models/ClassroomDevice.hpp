#pragma once

#include <string>
#include <vector>
#include "../third_party/nlohmann/json.hpp"

namespace smartclass {

struct ClassroomDevice {
    std::string id;
    std::string device_type; // "LIGHT", "FAN", "PROJECTOR"
    std::string name;        // e.g. "Front Classroom Lights"
    bool is_powered{false};  // ON / OFF
    int level{0};            // 0=off, 1-3=speed/brightness, etc.
    std::string location;    // e.g. "Front Board Area"

    friend void to_json(nlohmann::json& j, const ClassroomDevice& d) {
        j = nlohmann::json{
            {"id", d.id},
            {"device_type", d.device_type},
            {"name", d.name},
            {"is_powered", d.is_powered},
            {"level", d.level},
            {"location", d.location}
        };
    }

    friend void from_json(const nlohmann::json& j, ClassroomDevice& d) {
        if (j.contains("id") && !j["id"].is_null()) d.id = j["id"].get<std::string>();
        if (j.contains("device_type") && !j["device_type"].is_null()) d.device_type = j["device_type"].get<std::string>();
        if (j.contains("name") && !j["name"].is_null()) d.name = j["name"].get<std::string>();
        if (j.contains("is_powered") && !j["is_powered"].is_null()) d.is_powered = j["is_powered"].get<bool>();
        if (j.contains("level") && !j["level"].is_null()) d.level = j["level"].get<int>();
        if (j.contains("location") && !j["location"].is_null()) d.location = j["location"].get<std::string>();
    }
};

struct ClassroomState {
    std::string classroom_id{"CR-302"};
    std::string classroom_name{"Smart Class & IoT Lab 302"};
    double temperature_celsius{24.0};
    int occupancy_count{0};
    bool is_simulated{true};
    std::string last_telemetry_sync;
    std::vector<ClassroomDevice> devices;

    friend void to_json(nlohmann::json& j, const ClassroomState& s) {
        j = nlohmann::json{
            {"classroom_id", s.classroom_id},
            {"classroom_name", s.classroom_name},
            {"temperature_celsius", s.temperature_celsius},
            {"occupancy_count", s.occupancy_count},
            {"is_simulated", s.is_simulated},
            {"last_telemetry_sync", s.last_telemetry_sync},
            {"devices", s.devices}
        };
    }

    friend void from_json(const nlohmann::json& j, ClassroomState& s) {
        if (j.contains("classroom_id") && !j["classroom_id"].is_null()) s.classroom_id = j["classroom_id"].get<std::string>();
        if (j.contains("classroom_name") && !j["classroom_name"].is_null()) s.classroom_name = j["classroom_name"].get<std::string>();
        if (j.contains("temperature_celsius") && !j["temperature_celsius"].is_null()) s.temperature_celsius = j["temperature_celsius"].get<double>();
        if (j.contains("occupancy_count") && !j["occupancy_count"].is_null()) s.occupancy_count = j["occupancy_count"].get<int>();
        if (j.contains("is_simulated") && !j["is_simulated"].is_null()) s.is_simulated = j["is_simulated"].get<bool>();
        if (j.contains("last_telemetry_sync") && !j["last_telemetry_sync"].is_null()) s.last_telemetry_sync = j["last_telemetry_sync"].get<std::string>();
        if (j.contains("devices") && j["devices"].is_array()) s.devices = j["devices"].get<std::vector<ClassroomDevice>>();
    }
};

} // namespace smartclass
