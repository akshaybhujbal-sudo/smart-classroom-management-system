#include "../../include/storage/FileStorageManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace smartclass {

namespace fs = std::filesystem;

FileStorageManager::FileStorageManager(std::string data_dir)
    : data_dir_(std::move(data_dir)) {
    initializeStorage();
}

std::string FileStorageManager::getStudentsFilePath() const {
    return (fs::path(data_dir_) / "students.json").string();
}

std::string FileStorageManager::getAttendanceFilePath() const {
    return (fs::path(data_dir_) / "attendance.json").string();
}

std::string FileStorageManager::getTimetableFilePath() const {
    return (fs::path(data_dir_) / "timetable.json").string();
}

std::string FileStorageManager::getDevicesFilePath() const {
    return (fs::path(data_dir_) / "devices.json").string();
}

void FileStorageManager::initializeStorage() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!fs::exists(data_dir_)) {
        fs::create_directories(data_dir_);
    }

    createDefaultStudentsIfMissing();
    createDefaultAttendanceIfMissing();
    createDefaultTimetableIfMissing();
    createDefaultDevicesIfMissing();
}

void FileStorageManager::resetDemoData() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!fs::exists(data_dir_)) {
        fs::create_directories(data_dir_);
    }

    std::error_code ec;
    fs::remove(getStudentsFilePath(), ec);
    fs::remove(getAttendanceFilePath(), ec);
    fs::remove(getTimetableFilePath(), ec);
    fs::remove(getDevicesFilePath(), ec);

    createDefaultStudentsIfMissing();
    createDefaultAttendanceIfMissing();
    createDefaultTimetableIfMissing();
    createDefaultDevicesIfMissing();
}

bool FileStorageManager::atomicWriteFile(const std::string& target_path, const std::string& content) {
    std::string temp_path = target_path + ".tmp";
    {
        std::ofstream ofs(temp_path, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "[STORAGE ERROR] Unable to open temp file for writing: " << temp_path << "\n";
            return false;
        }
        ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
        ofs.flush();
        if (!ofs.good()) {
            std::cerr << "[STORAGE ERROR] Failed writing content to: " << temp_path << "\n";
            return false;
        }
    }

    try {
        if (fs::exists(target_path)) {
            fs::remove(target_path);
        }
        fs::rename(temp_path, target_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[STORAGE ERROR] Atomic rename failed: " << e.what() << "\n";
        // Fallback: copy and remove temp
        try {
            fs::copy_file(temp_path, target_path, fs::copy_options::overwrite_existing);
            fs::remove(temp_path);
            return true;
        } catch (...) {
            return false;
        }
    }
}

std::string FileStorageManager::readFileContent(const std::string& path) const {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("File not accessible: " + path);
    }
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// -----------------------------------------------------------------------------
// Student Storage
// -----------------------------------------------------------------------------
std::vector<Student> FileStorageManager::loadStudents() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::string path = getStudentsFilePath();
    if (!fs::exists(path)) {
        return {};
    }

    std::string content = readFileContent(path);
    if (content.empty()) return {};

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(content);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Malformed students.json: ") + e.what());
    }

    std::vector<Student> result;
    if (j.contains("students") && j["students"].is_array()) {
        result = j["students"].get<std::vector<Student>>();
    }
    return result;
}

bool FileStorageManager::saveStudents(const std::vector<Student>& students) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    nlohmann::json j;
    j["students"] = students;
    return atomicWriteFile(getStudentsFilePath(), j.dump(2));
}

// -----------------------------------------------------------------------------
// Attendance Storage
// -----------------------------------------------------------------------------
std::vector<AttendanceRecord> FileStorageManager::loadAttendance() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::string path = getAttendanceFilePath();
    if (!fs::exists(path)) {
        return {};
    }

    std::string content = readFileContent(path);
    if (content.empty()) return {};

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(content);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Malformed attendance.json: ") + e.what());
    }

    std::vector<AttendanceRecord> result;
    if (j.contains("records") && j["records"].is_array()) {
        result = j["records"].get<std::vector<AttendanceRecord>>();
    }
    return result;
}

bool FileStorageManager::saveAttendance(const std::vector<AttendanceRecord>& records) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    nlohmann::json j;
    j["records"] = records;
    return atomicWriteFile(getAttendanceFilePath(), j.dump(2));
}

// -----------------------------------------------------------------------------
// Timetable Storage
// -----------------------------------------------------------------------------
std::vector<TimetableEntry> FileStorageManager::loadTimetable() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::string path = getTimetableFilePath();
    if (!fs::exists(path)) {
        return {};
    }

    std::string content = readFileContent(path);
    if (content.empty()) return {};

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(content);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Malformed timetable.json: ") + e.what());
    }

    std::vector<TimetableEntry> result;
    if (j.contains("schedule") && j["schedule"].is_array()) {
        result = j["schedule"].get<std::vector<TimetableEntry>>();
    }
    return result;
}

bool FileStorageManager::saveTimetable(const std::vector<TimetableEntry>& entries) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    nlohmann::json j;
    j["schedule"] = entries;
    return atomicWriteFile(getTimetableFilePath(), j.dump(2));
}

// -----------------------------------------------------------------------------
// Classroom Device State Storage
// -----------------------------------------------------------------------------
ClassroomState FileStorageManager::loadClassroomState() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::string path = getDevicesFilePath();
    if (!fs::exists(path)) {
        return ClassroomState{};
    }

    std::string content = readFileContent(path);
    if (content.empty()) return ClassroomState{};

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(content);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Malformed devices.json: ") + e.what());
    }

    return j.get<ClassroomState>();
}

bool FileStorageManager::saveClassroomState(const ClassroomState& state) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    nlohmann::json j = state;
    return atomicWriteFile(getDevicesFilePath(), j.dump(2));
}

// -----------------------------------------------------------------------------
// Default Demo Data Initializers (Only run if file does not exist)
// -----------------------------------------------------------------------------
void FileStorageManager::createDefaultStudentsIfMissing() {
    std::string path = getStudentsFilePath();
    if (fs::exists(path)) return;

    // 10+ Fictional students clearly documented for college demonstration
    nlohmann::json j = {
        {"students", nlohmann::json::array({
            {{"id", "std-001"}, {"roll_number", "CS2026-001"}, {"full_name", "Aarav Sharma"}, {"division", "A"}, {"department", "Computer Science"}, {"email", "aarav.sharma@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:00:00Z"}},
            {{"id", "std-002"}, {"roll_number", "CS2026-002"}, {"full_name", "Diya Patel"}, {"division", "A"}, {"department", "Computer Science"}, {"email", "diya.patel@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:05:00Z"}},
            {{"id", "std-003"}, {"roll_number", "CS2026-003"}, {"full_name", "Rohan Verma"}, {"division", "A"}, {"department", "Computer Science"}, {"email", "rohan.verma@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:10:00Z"}},
            {{"id", "std-004"}, {"roll_number", "CS2026-004"}, {"full_name", "Ananya Iyer"}, {"division", "A"}, {"department", "Computer Science"}, {"email", "ananya.iyer@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:15:00Z"}},
            {{"id", "std-005"}, {"roll_number", "CS2026-005"}, {"full_name", "Vikram Singh"}, {"division", "B"}, {"department", "Computer Science"}, {"email", "vikram.singh@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:20:00Z"}},
            {{"id", "std-006"}, {"roll_number", "CS2026-006"}, {"full_name", "Sneha Kulkarni"}, {"division", "B"}, {"department", "Computer Science"}, {"email", "sneha.k@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:25:00Z"}},
            {{"id", "std-007"}, {"roll_number", "CS2026-007"}, {"full_name", "Arjun Nair"}, {"division", "B"}, {"department", "Computer Science"}, {"email", "arjun.nair@college.edu"}, {"semester", 6}, {"is_active", false}, {"created_at", "2026-10-01T09:30:00Z"}},
            {{"id", "std-008"}, {"roll_number", "CS2026-008"}, {"full_name", "Meera Joshi"}, {"division", "A"}, {"department", "Computer Science"}, {"email", "meera.joshi@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:35:00Z"}},
            {{"id", "std-009"}, {"roll_number", "CS2026-009"}, {"full_name", "Kabir Mehta"}, {"division", "B"}, {"department", "Computer Science"}, {"email", "kabir.mehta@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:40:00Z"}},
            {{"id", "std-010"}, {"roll_number", "CS2026-010"}, {"full_name", "Pooja Reddy"}, {"division", "A"}, {"department", "Computer Science"}, {"email", "pooja.reddy@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:45:00Z"}},
            {{"id", "std-011"}, {"roll_number", "CS2026-011"}, {"full_name", "Siddharth Rao"}, {"division", "B"}, {"department", "Computer Science"}, {"email", "siddharth.r@college.edu"}, {"semester", 6}, {"is_active", true}, {"created_at", "2026-10-01T09:50:00Z"}}
        })}
    };
    atomicWriteFile(path, j.dump(2));
}

void FileStorageManager::createDefaultAttendanceIfMissing() {
    std::string path = getAttendanceFilePath();
    if (fs::exists(path)) return;

    nlohmann::json j = {
        {"records", nlohmann::json::array({
            {{"id", "att-20261019-CS2026-001"}, {"date", "2026-10-19"}, {"student_id", "std-001"}, {"student_roll", "CS2026-001"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-19T09:30:00Z"}, {"remarks", "On time"}},
            {{"id", "att-20261019-CS2026-002"}, {"date", "2026-10-19"}, {"student_id", "std-002"}, {"student_roll", "CS2026-002"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-19T09:30:00Z"}, {"remarks", "On time"}},
            {{"id", "att-20261019-CS2026-003"}, {"date", "2026-10-19"}, {"student_id", "std-003"}, {"student_roll", "CS2026-003"}, {"status", "ABSENT"}, {"recorded_at", "2026-10-19T09:30:00Z"}, {"remarks", "Medical leave"}},
            {{"id", "att-20261020-CS2026-001"}, {"date", "2026-10-20"}, {"student_id", "std-001"}, {"student_roll", "CS2026-001"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-20T09:30:00Z"}, {"remarks", "On time"}},
            {{"id", "att-20261020-CS2026-002"}, {"date", "2026-10-20"}, {"student_id", "std-002"}, {"student_roll", "CS2026-002"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-20T09:30:00Z"}, {"remarks", "On time"}},
            {{"id", "att-20261020-CS2026-003"}, {"date", "2026-10-20"}, {"student_id", "std-003"}, {"student_roll", "CS2026-003"}, {"status", "ABSENT"}, {"recorded_at", "2026-10-20T09:30:00Z"}, {"remarks", "Unexcused"}},
            {{"id", "att-20261021-CS2026-001"}, {"date", "2026-10-21"}, {"student_id", "std-001"}, {"student_roll", "CS2026-001"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-21T09:30:00Z"}, {"remarks", "On time"}},
            {{"id", "att-20261021-CS2026-002"}, {"date", "2026-10-21"}, {"student_id", "std-002"}, {"student_roll", "CS2026-002"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-21T09:30:00Z"}, {"remarks", "On time"}},
            {{"id", "att-20261021-CS2026-003"}, {"date", "2026-10-21"}, {"student_id", "std-003"}, {"student_roll", "CS2026-003"}, {"status", "PRESENT"}, {"recorded_at", "2026-10-21T09:30:00Z"}, {"remarks", "Returned"}}
        })}
    };
    atomicWriteFile(path, j.dump(2));
}

void FileStorageManager::createDefaultTimetableIfMissing() {
    std::string path = getTimetableFilePath();
    if (fs::exists(path)) return;

    nlohmann::json j = {
        {"schedule", nlohmann::json::array({
            {{"id", "tt-101"}, {"subject", "CS601: Distributed Systems"}, {"teacher", "Dr. Priya Nair"}, {"classroom", "CR-302 (Smart Hall)"}, {"weekday", "MONDAY"}, {"start_time", "09:00"}, {"end_time", "10:00"}},
            {{"id", "tt-102"}, {"subject", "CS602: Compiler Design"}, {"teacher", "Prof. R. Sen"}, {"classroom", "CR-302 (Smart Hall)"}, {"weekday", "MONDAY"}, {"start_time", "10:15"}, {"end_time", "11:15"}},
            {{"id", "tt-201"}, {"subject", "CS603: Computer Networks"}, {"teacher", "Dr. V. Raman"}, {"classroom", "CR-302 (Smart Hall)"}, {"weekday", "TUESDAY"}, {"start_time", "09:00"}, {"end_time", "10:00"}},
            {{"id", "tt-301"}, {"subject", "CS604: Internet of Things"}, {"teacher", "Prof. S. Joshi"}, {"classroom", "CR-302 (Smart Hall)"}, {"weekday", "WEDNESDAY"}, {"start_time", "09:30"}, {"end_time", "10:30"}}
        })}
    };
    atomicWriteFile(path, j.dump(2));
}

void FileStorageManager::createDefaultDevicesIfMissing() {
    std::string path = getDevicesFilePath();
    if (fs::exists(path)) return;

    nlohmann::json j = {
        {"classroom_id", "CR-302"},
        {"classroom_name", "Smart Class & IoT Lab 302"},
        {"temperature_celsius", 23.8},
        {"occupancy_count", 28},
        {"is_simulated", true},
        {"last_telemetry_sync", "2026-10-21T09:45:00Z"},
        {"devices", nlohmann::json::array({
            {{"id", "dev-light-01"}, {"device_type", "LIGHT"}, {"name", "Front Classroom Lights"}, {"is_powered", true}, {"level", 1}, {"location", "Front Board Area"}},
            {{"id", "dev-light-02"}, {"device_type", "LIGHT"}, {"name", "Rear Seating Lights"}, {"is_powered", true}, {"level", 1}, {"location", "Student Seating Area"}},
            {{"id", "dev-fan-01"}, {"device_type", "FAN"}, {"name", "Ceiling Ventilation Fan Bank"}, {"is_powered", true}, {"level", 2}, {"location", "Ceiling Center"}},
            {{"id", "dev-proj-01"}, {"device_type", "PROJECTOR"}, {"name", "Smart Interactive HD Projector"}, {"is_powered", false}, {"level", 0}, {"location", "Ceiling Mount Screen"}}
        })}
    };
    atomicWriteFile(path, j.dump(2));
}

} // namespace smartclass
