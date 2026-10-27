#include "../../include/services/ClassroomManager.hpp"
#include "../../include/utils/Validation.hpp"
#include <algorithm>
#include <cmath>

namespace smartclass {

ClassroomManager::ClassroomManager(std::shared_ptr<FileStorageManager> storage)
    : storage_(std::move(storage)) {}

ServiceResult<ClassroomState> ClassroomManager::getClassroomStatus() {
    auto state = storage_->loadClassroomState();
    return ServiceResult<ClassroomState>::ok(state, 200);
}

ServiceResult<ClassroomDevice> ClassroomManager::getDeviceById(const std::string& device_id) {
    std::string clean_id = utils::trim(device_id);
    if (clean_id.empty()) {
        return ServiceResult<ClassroomDevice>::fail("Device ID cannot be empty.", 400);
    }

    auto state = storage_->loadClassroomState();
    for (const auto& d : state.devices) {
        if (d.id == clean_id) {
            return ServiceResult<ClassroomDevice>::ok(d, 200);
        }
    }
    return ServiceResult<ClassroomDevice>::fail("Device with ID '" + clean_id + "' not found.", 404);
}

ServiceResult<ClassroomState> ClassroomManager::updateDeviceState(const std::string& device_id, bool is_powered, int level) {
    std::string clean_id = utils::trim(device_id);
    if (clean_id.empty()) {
        return ServiceResult<ClassroomState>::fail("Device ID cannot be empty.", 400);
    }
    if (level < 0 || level > 10) {
        return ServiceResult<ClassroomState>::fail("Device level setting must be between 0 and 10.", 400);
    }

    auto state = storage_->loadClassroomState();
    auto it = std::find_if(state.devices.begin(), state.devices.end(), [&](const ClassroomDevice& d) {
        return d.id == clean_id;
    });

    if (it == state.devices.end()) {
        return ServiceResult<ClassroomState>::fail("Device with ID '" + clean_id + "' not found.", 404);
    }

    it->is_powered = is_powered;
    it->level = is_powered ? (level == 0 ? 1 : level) : 0;
    state.last_telemetry_sync = utils::getCurrentTimestamp();

    if (!storage_->saveClassroomState(state)) {
        return ServiceResult<ClassroomState>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<ClassroomState>::ok(state, 200);
}

ServiceResult<ClassroomState> ClassroomManager::updateTelemetry(double temperature, int occupancy) {
    if (temperature < -20.0 || temperature > 70.0) {
        return ServiceResult<ClassroomState>::fail("Temperature value out of realistic sensor range (-20 to 70 C).", 400);
    }
    if (occupancy < 0) {
        return ServiceResult<ClassroomState>::fail("Occupancy count cannot be negative.", 400);
    }

    auto state = storage_->loadClassroomState();
    state.temperature_celsius = std::round(temperature * 10.0) / 10.0;
    state.occupancy_count = occupancy;
    state.last_telemetry_sync = utils::getCurrentTimestamp();

    if (!storage_->saveClassroomState(state)) {
        return ServiceResult<ClassroomState>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<ClassroomState>::ok(state, 200);
}

ServiceResult<ClassroomState> ClassroomManager::simulateTick() {
    auto state = storage_->loadClassroomState();
    // Simulate slight natural variation
    static double drift = 0.1;
    state.temperature_celsius += drift;
    if (state.temperature_celsius > 26.0) drift = -0.1;
    if (state.temperature_celsius < 22.0) drift = 0.1;
    state.temperature_celsius = std::round(state.temperature_celsius * 10.0) / 10.0;
    state.last_telemetry_sync = utils::getCurrentTimestamp();

    storage_->saveClassroomState(state);
    return ServiceResult<ClassroomState>::ok(state, 200);
}

} // namespace smartclass
