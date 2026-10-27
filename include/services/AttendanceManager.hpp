#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../models/AttendanceRecord.hpp"
#include "../storage/FileStorageManager.hpp"
#include "../services/StudentManager.hpp"
#include "../utils/ServiceResult.hpp"

namespace smartclass {

struct StudentAttendanceStat {
    std::string student_id;
    std::string roll_number;
    std::string full_name;
    int total_sessions{0};
    int attended_sessions{0};
    int absent_sessions{0};
    double attendance_percentage{0.0};
    bool is_defaulter{false}; // True if < 75% and total_sessions > 0
    bool is_active{true};

    friend void to_json(nlohmann::json& j, const StudentAttendanceStat& s) {
        j = nlohmann::json{
            {"student_id", s.student_id},
            {"roll_number", s.roll_number},
            {"full_name", s.full_name},
            {"total_sessions", s.total_sessions},
            {"attended_sessions", s.attended_sessions},
            {"absent_sessions", s.absent_sessions},
            {"attendance_percentage", s.attendance_percentage},
            {"is_defaulter", s.is_defaulter},
            {"is_active", s.is_active}
        };
    }
};

struct DailyAttendanceSummary {
    std::string date;
    int total_students{0};
    int present_count{0};
    int absent_count{0};
    double attendance_percentage{0.0};

    friend void to_json(nlohmann::json& j, const DailyAttendanceSummary& s) {
        j = nlohmann::json{
            {"date", s.date},
            {"total_students", s.total_students},
            {"present_count", s.present_count},
            {"absent_count", s.absent_count},
            {"attendance_percentage", s.attendance_percentage}
        };
    }
};

class AttendanceManager {
public:
    AttendanceManager(std::shared_ptr<FileStorageManager> storage,
                      std::shared_ptr<StudentManager> student_manager);

    // Business Operations
    ServiceResult<AttendanceRecord> recordAttendance(AttendanceRecord record);
    ServiceResult<std::vector<AttendanceRecord>> getAttendanceByDate(const std::string& date);
    ServiceResult<std::vector<AttendanceRecord>> getStudentAttendanceHistory(const std::string& student_id_or_roll);
    ServiceResult<StudentAttendanceStat> calculateStudentAttendancePercentage(const std::string& student_id_or_roll);
    ServiceResult<std::vector<StudentAttendanceStat>> getAllStudentsAttendanceStats();
    ServiceResult<DailyAttendanceSummary> getDailyAttendanceSummary(const std::string& date);
    ServiceResult<AttendanceRecord> updateAttendance(const std::string& record_id, const std::string& status, const std::string& remarks);

private:
    std::shared_ptr<FileStorageManager> storage_;
    std::shared_ptr<StudentManager> student_manager_;

    std::string generateAttendanceId(const std::string& date, const std::string& roll);
    ServiceResult<void> validateAttendanceFields(const AttendanceRecord& r);
};

} // namespace smartclass
