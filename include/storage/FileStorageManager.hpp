#pragma once

#include <string>
#include <vector>
#include <shared_mutex>
#include <filesystem>
#include "../models/Student.hpp"
#include "../models/AttendanceRecord.hpp"
#include "../models/TimetableEntry.hpp"
#include "../models/ClassroomDevice.hpp"

namespace smartclass {

class FileStorageManager {
public:
    explicit FileStorageManager(std::string data_dir = "data");
    virtual ~FileStorageManager() = default;

    // Initialization & integrity
    void initializeStorage();
    void resetDemoData();

    // Student Storage
    std::vector<Student> loadStudents();
    bool saveStudents(const std::vector<Student>& students);

    // Attendance Storage
    std::vector<AttendanceRecord> loadAttendance();
    bool saveAttendance(const std::vector<AttendanceRecord>& records);

    // Timetable Storage
    std::vector<TimetableEntry> loadTimetable();
    bool saveTimetable(const std::vector<TimetableEntry>& entries);

    // Classroom Device State Storage
    ClassroomState loadClassroomState();
    bool saveClassroomState(const ClassroomState& state);

    // Helper to get active directory path
    std::string getDataDirectory() const { return data_dir_; }

private:
    std::string data_dir_;
    mutable std::shared_mutex mutex_;

    std::string getStudentsFilePath() const;
    std::string getAttendanceFilePath() const;
    std::string getTimetableFilePath() const;
    std::string getDevicesFilePath() const;

    bool atomicWriteFile(const std::string& target_path, const std::string& content);
    std::string readFileContent(const std::string& path) const;

    void createDefaultStudentsIfMissing();
    void createDefaultAttendanceIfMissing();
    void createDefaultTimetableIfMissing();
    void createDefaultDevicesIfMissing();
};

} // namespace smartclass
