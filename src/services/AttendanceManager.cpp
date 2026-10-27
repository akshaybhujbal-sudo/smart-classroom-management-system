#include "../../include/services/AttendanceManager.hpp"
#include "../../include/utils/Validation.hpp"
#include <algorithm>
#include <cmath>

namespace smartclass {

AttendanceManager::AttendanceManager(std::shared_ptr<FileStorageManager> storage,
                                     std::shared_ptr<StudentManager> student_manager)
    : storage_(std::move(storage)), student_manager_(std::move(student_manager)) {}

std::string AttendanceManager::generateAttendanceId(const std::string& date, const std::string& roll) {
    std::string clean_date = date;
    clean_date.erase(std::remove(clean_date.begin(), clean_date.end(), '-'), clean_date.end());
    return "att-" + clean_date + "-" + roll;
}

ServiceResult<void> AttendanceManager::validateAttendanceFields(const AttendanceRecord& r) {
    if (!utils::isValidDate(r.date)) {
        return ServiceResult<void>::fail("Invalid date format. Expected YYYY-MM-DD representing a valid calendar date.", 400);
    }

    std::string status_upper = utils::toUpper(utils::trim(r.status));
    if (status_upper != "PRESENT" && status_upper != "ABSENT") {
        return ServiceResult<void>::fail("Invalid attendance status. Status must be 'PRESENT' or 'ABSENT'.", 400);
    }

    return ServiceResult<void>::ok();
}

ServiceResult<AttendanceRecord> AttendanceManager::recordAttendance(AttendanceRecord record) {
    record.date = utils::trim(record.date);
    record.status = utils::toUpper(utils::trim(record.status));
    record.remarks = utils::trim(record.remarks);

    auto val_res = validateAttendanceFields(record);
    if (!val_res.success) {
        return ServiceResult<AttendanceRecord>::fail(val_res.error_message, val_res.http_status);
    }

    // Resolve and validate student
    Student student;
    if (!record.student_id.empty()) {
        auto s_res = student_manager_->getStudentById(record.student_id);
        if (!s_res.success) {
            // Try resolving by roll number in case ID was passed as roll
            auto s_roll_res = student_manager_->getStudentByRollNumber(record.student_id);
            if (!s_roll_res.success) {
                return ServiceResult<AttendanceRecord>::fail("Student record not found for ID/Roll: " + record.student_id, 404);
            }
            student = *s_roll_res.data;
        } else {
            student = *s_res.data;
        }
    } else if (!record.student_roll.empty()) {
        auto s_res = student_manager_->getStudentByRollNumber(record.student_roll);
        if (!s_res.success) {
            return ServiceResult<AttendanceRecord>::fail("Student with roll number '" + record.student_roll + "' not found.", 404);
        }
        student = *s_res.data;
    } else {
        return ServiceResult<AttendanceRecord>::fail("Either student_id or student_roll must be provided.", 400);
    }

    // Rule: Reject attendance for deactivated / inactive students
    if (!student.is_active) {
        return ServiceResult<AttendanceRecord>::fail(
            "Cannot record attendance for deactivated/inactive student (" + student.roll_number + ").", 400);
    }

    // Ensure consistent IDs
    record.student_id = student.id;
    record.student_roll = student.roll_number;

    auto all_records = storage_->loadAttendance();

    // Check for existing record on the same date for this student
    // Business rule: Prevent duplicate records by performing an idempotent update
    auto it = std::find_if(all_records.begin(), all_records.end(), [&](const AttendanceRecord& existing) {
        return existing.date == record.date &&
               (existing.student_id == record.student_id ||
                utils::toUpper(existing.student_roll) == utils::toUpper(record.student_roll));
    });

    if (it != all_records.end()) {
        // Idempotent update of existing record
        it->status = record.status;
        it->remarks = record.remarks;
        it->recorded_at = utils::getCurrentTimestamp();
        record = *it;
    } else {
        // New record
        if (record.id.empty()) {
            record.id = generateAttendanceId(record.date, record.student_roll);
        }
        record.recorded_at = utils::getCurrentTimestamp();
        all_records.push_back(record);
    }

    if (!storage_->saveAttendance(all_records)) {
        return ServiceResult<AttendanceRecord>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<AttendanceRecord>::ok(record, 200);
}

ServiceResult<std::vector<AttendanceRecord>> AttendanceManager::getAttendanceByDate(const std::string& date) {
    std::string clean_date = utils::trim(date);
    if (!utils::isValidDate(clean_date)) {
        return ServiceResult<std::vector<AttendanceRecord>>::fail(
            "Invalid date format. Expected YYYY-MM-DD.", 400);
    }

    auto all_records = storage_->loadAttendance();
    std::vector<AttendanceRecord> filtered;
    for (const auto& r : all_records) {
        if (r.date == clean_date) {
            filtered.push_back(r);
        }
    }
    return ServiceResult<std::vector<AttendanceRecord>>::ok(filtered, 200);
}

ServiceResult<std::vector<AttendanceRecord>> AttendanceManager::getStudentAttendanceHistory(const std::string& student_id_or_roll) {
    std::string target = utils::trim(student_id_or_roll);
    if (target.empty()) {
        return ServiceResult<std::vector<AttendanceRecord>>::fail("Student identifier cannot be empty.", 400);
    }

    std::string target_upper = utils::toUpper(target);
    auto all_records = storage_->loadAttendance();
    std::vector<AttendanceRecord> history;

    for (const auto& r : all_records) {
        if (r.student_id == target || utils::toUpper(r.student_roll) == target_upper) {
            history.push_back(r);
        }
    }
    return ServiceResult<std::vector<AttendanceRecord>>::ok(history, 200);
}

ServiceResult<StudentAttendanceStat> AttendanceManager::calculateStudentAttendancePercentage(const std::string& student_id_or_roll) {
    std::string target = utils::trim(student_id_or_roll);
    if (target.empty()) {
        return ServiceResult<StudentAttendanceStat>::fail("Student identifier cannot be empty.", 400);
    }

    // Try resolving student
    Student student;
    auto s_res = student_manager_->getStudentById(target);
    if (s_res.success) {
        student = *s_res.data;
    } else {
        auto roll_res = student_manager_->getStudentByRollNumber(target);
        if (roll_res.success) {
            student = *roll_res.data;
        } else {
            return ServiceResult<StudentAttendanceStat>::fail("Student '" + target + "' not found.", 404);
        }
    }

    auto history_res = getStudentAttendanceHistory(student.roll_number);
    auto history = history_res.data.value_or(std::vector<AttendanceRecord>{});

    StudentAttendanceStat stat;
    stat.student_id = student.id;
    stat.roll_number = student.roll_number;
    stat.full_name = student.full_name;
    stat.is_active = student.is_active;
    stat.total_sessions = static_cast<int>(history.size());

    int present_count = 0;
    int absent_count = 0;
    for (const auto& r : history) {
        if (utils::toUpper(r.status) == "PRESENT") {
            present_count++;
        } else {
            absent_count++;
        }
    }
    stat.attended_sessions = present_count;
    stat.absent_sessions = absent_count;

    // Attendance percentage calculation formula:
    // (Attended Sessions / Total Sessions) * 100
    // Zero-division protection: If total_sessions is 0, percentage is strictly 0.0
    if (stat.total_sessions > 0) {
        double raw_pct = (static_cast<double>(stat.attended_sessions) / stat.total_sessions) * 100.0;
        stat.attendance_percentage = std::round(raw_pct * 10.0) / 10.0; // 1 decimal place
        stat.is_defaulter = (stat.attendance_percentage < 75.0);
    } else {
        stat.attendance_percentage = 0.0;
        stat.is_defaulter = false;
    }

    return ServiceResult<StudentAttendanceStat>::ok(stat, 200);
}

ServiceResult<std::vector<StudentAttendanceStat>> AttendanceManager::getAllStudentsAttendanceStats() {
    auto students_res = student_manager_->getAllStudents(true);
    if (!students_res.success) {
        return ServiceResult<std::vector<StudentAttendanceStat>>::fail("Failed to retrieve students.", 500);
    }

    std::vector<StudentAttendanceStat> stats;
    for (const auto& s : *students_res.data) {
        auto stat_res = calculateStudentAttendancePercentage(s.id);
        if (stat_res.success) {
            stats.push_back(*stat_res.data);
        }
    }
    return ServiceResult<std::vector<StudentAttendanceStat>>::ok(stats, 200);
}

ServiceResult<DailyAttendanceSummary> AttendanceManager::getDailyAttendanceSummary(const std::string& date) {
    std::string clean_date = utils::trim(date);
    if (!utils::isValidDate(clean_date)) {
        return ServiceResult<DailyAttendanceSummary>::fail("Invalid date format. Expected YYYY-MM-DD.", 400);
    }

    auto records_res = getAttendanceByDate(clean_date);
    if (!records_res.success) {
        return ServiceResult<DailyAttendanceSummary>::fail("Failed to load records for date.", 500);
    }

    DailyAttendanceSummary summary;
    summary.date = clean_date;
    summary.total_students = static_cast<int>(records_res.data->size());

    for (const auto& r : *records_res.data) {
        if (utils::toUpper(r.status) == "PRESENT") {
            summary.present_count++;
        } else {
            summary.absent_count++;
        }
    }

    if (summary.total_students > 0) {
        double raw_pct = (static_cast<double>(summary.present_count) / summary.total_students) * 100.0;
        summary.attendance_percentage = std::round(raw_pct * 10.0) / 10.0;
    } else {
        summary.attendance_percentage = 0.0;
    }

    return ServiceResult<DailyAttendanceSummary>::ok(summary, 200);
}

ServiceResult<AttendanceRecord> AttendanceManager::updateAttendance(const std::string& record_id,
                                                                   const std::string& status,
                                                                   const std::string& remarks) {
    std::string clean_id = utils::trim(record_id);
    std::string clean_status = utils::toUpper(utils::trim(status));

    if (clean_id.empty()) {
        return ServiceResult<AttendanceRecord>::fail("Attendance record ID cannot be empty.", 400);
    }
    if (clean_status != "PRESENT" && clean_status != "ABSENT") {
        return ServiceResult<AttendanceRecord>::fail("Status must be 'PRESENT' or 'ABSENT'.", 400);
    }

    auto all_records = storage_->loadAttendance();
    auto it = std::find_if(all_records.begin(), all_records.end(), [&](const AttendanceRecord& r) {
        return r.id == clean_id;
    });

    if (it == all_records.end()) {
        return ServiceResult<AttendanceRecord>::fail("Attendance record with ID '" + clean_id + "' not found.", 404);
    }

    it->status = clean_status;
    it->remarks = utils::trim(remarks);
    it->recorded_at = utils::getCurrentTimestamp();

    if (!storage_->saveAttendance(all_records)) {
        return ServiceResult<AttendanceRecord>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<AttendanceRecord>::ok(*it, 200);
}

} // namespace smartclass
