#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../models/ClassroomDevice.hpp"
#include "../storage/FileStorageManager.hpp"
#include "../utils/ServiceResult.hpp"

namespace smartclass {

class ClassroomManager {
public:
    explicit ClassroomManager(std::shared_ptr<FileStorageManager> storage);

    // Business Operations
    ServiceResult<ClassroomState> getClassroomStatus();
    ServiceResult<ClassroomDevice> getDeviceById(const std::string& device_id);
    ServiceResult<ClassroomState> updateDeviceState(const std::string& device_id, bool is_powered, int level = 0);
    ServiceResult<ClassroomState> updateTelemetry(double temperature, int occupancy);
    ServiceResult<ClassroomState> simulateTick(); // Small ambient variation for demo

private:
    std::shared_ptr<FileStorageManager> storage_;
};

} // namespace smartclass
