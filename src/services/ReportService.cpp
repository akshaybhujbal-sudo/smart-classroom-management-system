#include "../../include/services/ReportService.hpp"
#include "../../include/utils/Validation.hpp"
#include "../../include/utils/CsvExporter.hpp"
#include <sstream>
#include <set>

namespace smartclass {

ReportService::ReportService(std::shared_ptr<FileStorageManager> storage,
                             std::shared_ptr<StudentManager> student_mgr,
                             std::shared_ptr<AttendanceManager> attendance_mgr,
                             std::shared_ptr<TimetableManager> timetable_mgr,
                             std::shared_ptr<ClassroomManager> classroom_mgr)
    : storage_(std::move(storage)),
      student_mgr_(std::move(student_mgr)),
      attendance_mgr_(std::move(attendance_mgr)),
      timetable_mgr_(std::move(timetable_mgr)),
      classroom_mgr_(std::move(classroom_mgr)) {}

ServiceResult<DailyAttendanceSummary> ReportService::getDailyAttendanceSummary(const std::string& date) {
    return attendance_mgr_->getDailyAttendanceSummary(date);
}

ServiceResult<std::vector<StudentAttendanceStat>> ReportService::getStudentStatisticsReport() {
    return attendance_mgr_->getAllStudentsAttendanceStats();
}

ServiceResult<std::vector<StudentAttendanceStat>> ReportService::getDefaultersReport(double threshold) {
    auto stats_res = attendance_mgr_->getAllStudentsAttendanceStats();
    if (!stats_res.success) {
        return stats_res;
    }

    std::vector<StudentAttendanceStat> defaulters;
    for (const auto& stat : *stats_res.data) {
        if (stat.total_sessions > 0 && stat.attendance_percentage < threshold) {
            defaulters.push_back(stat);
        }
    }
    return ServiceResult<std::vector<StudentAttendanceStat>>::ok(defaulters, 200);
}

ServiceResult<nlohmann::json> ReportService::getClassroomStatusSummary() {
    auto state_res = classroom_mgr_->getClassroomStatus();
    if (!state_res.success) {
        return ServiceResult<nlohmann::json>::fail("Failed to retrieve classroom state.", 500);
    }

    const auto& state = *state_res.data;
    int powered_on = 0;
    int lights_on = 0;
    int fans_on = 0;
    bool projector_on = false;

    for (const auto& d : state.devices) {
        if (d.is_powered) {
            powered_on++;
            if (d.device_type == "LIGHT") lights_on++;
            else if (d.device_type == "FAN") fans_on++;
            else if (d.device_type == "PROJECTOR") projector_on = true;
        }
    }

    nlohmann::json summary = {
        {"classroom_id", state.classroom_id},
        {"classroom_name", state.classroom_name},
        {"temperature_celsius", state.temperature_celsius},
        {"occupancy_count", state.occupancy_count},
        {"is_simulated", state.is_simulated},
        {"total_devices", state.devices.size()},
        {"active_devices", powered_on},
        {"lights_on", lights_on},
        {"fans_on", fans_on},
        {"projector_on", projector_on},
        {"last_sync", state.last_telemetry_sync}
    };

    return ServiceResult<nlohmann::json>::ok(summary, 200);
}

ServiceResult<nlohmann::json> ReportService::getDashboardSummary() {
    auto students_res = student_mgr_->getAllStudents(true);
    int total_students = 0;
    int active_students = 0;
    int inactive_students = 0;
    if (students_res.success) {
        total_students = static_cast<int>(students_res.data->size());
        for (const auto& s : *students_res.data) {
            if (s.is_active) active_students++;
            else inactive_students++;
        }
    }

    // Today's date and attendance
    std::string today = utils::getCurrentDate();
    auto today_att_res = attendance_mgr_->getDailyAttendanceSummary(today);
    DailyAttendanceSummary today_att;
    if (today_att_res.success) {
        today_att = *today_att_res.data;
    }

    // Historical attendance records to build trend & latest session fallback
    auto all_att = storage_->loadAttendance();
    std::set<std::string> distinct_dates;
    for (const auto& r : all_att) {
        if (!r.date.empty()) distinct_dates.insert(r.date);
    }

    std::vector<std::string> sorted_dates(distinct_dates.begin(), distinct_dates.end());
    nlohmann::json trend_array = nlohmann::json::array();
    DailyAttendanceSummary latest_att;
    bool has_latest = false;

    if (!sorted_dates.empty()) {
        std::string latest_date = sorted_dates.back();
        auto latest_res = attendance_mgr_->getDailyAttendanceSummary(latest_date);
        if (latest_res.success) {
            latest_att = *latest_res.data;
            has_latest = true;
        }

        // Keep last 5 dates for trend visualization
        size_t start_idx = (sorted_dates.size() > 5) ? (sorted_dates.size() - 5) : 0;
        for (size_t i = start_idx; i < sorted_dates.size(); ++i) {
            auto d_res = attendance_mgr_->getDailyAttendanceSummary(sorted_dates[i]);
            if (d_res.success) {
                trend_array.push_back({
                    {"date", sorted_dates[i]},
                    {"percentage", d_res.data->attendance_percentage},
                    {"present", d_res.data->present_count},
                    {"absent", d_res.data->absent_count},
                    {"total", d_res.data->total_students}
                });
            }
        }
    }

    // Classroom summary
    auto class_summary_res = getClassroomStatusSummary();
    nlohmann::json class_summary = class_summary_res.success ? *class_summary_res.data : nlohmann::json::object();

    // Defaulters count
    auto defaulters_res = getDefaultersReport(75.0);
    int defaulters_count = defaulters_res.success ? static_cast<int>(defaulters_res.data->size()) : 0;

    nlohmann::json dashboard = {
        {"system_status", "ONLINE"},
        {"timestamp", utils::getCurrentTimestamp()},
        {"current_date", today},
        {"students", {
            {"total", total_students},
            {"active", active_students},
            {"inactive", inactive_students},
            {"defaulters_count", defaulters_count}
        }},
        {"today_attendance", {
            {"total_logged", today_att.total_students},
            {"present", today_att.present_count},
            {"absent", today_att.absent_count},
            {"percentage", today_att.attendance_percentage}
        }},
        {"latest_attendance", has_latest ? nlohmann::json({
            {"date", latest_att.date},
            {"total_logged", latest_att.total_students},
            {"present", latest_att.present_count},
            {"absent", latest_att.absent_count},
            {"percentage", latest_att.attendance_percentage}
        }) : nullptr},
        {"attendance_trend", trend_array},
        {"classroom", class_summary}
    };

    return ServiceResult<nlohmann::json>::ok(dashboard, 200);
}

ServiceResult<std::string> ReportService::exportAttendanceCsv(const std::string& date_filter) {
    auto records = storage_->loadAttendance();
    std::string clean_filter = utils::trim(date_filter);

    std::ostringstream csv;
    // Header row
    csv << utils::CsvExporter::buildRow({
        "Attendance Record ID",
        "Session Date",
        "Student ID",
        "Roll Number",
        "Status",
        "Recorded At",
        "Remarks"
    });

    for (const auto& r : records) {
        if (!clean_filter.empty() && r.date != clean_filter) {
            continue;
        }
        csv << utils::CsvExporter::buildRow({
            r.id,
            r.date,
            r.student_id,
            r.student_roll,
            r.status,
            r.recorded_at,
            r.remarks
        });
    }

    return ServiceResult<std::string>::ok(csv.str(), 200);
}

ServiceResult<std::string> ReportService::exportStudentsCsv() {
    auto students = storage_->loadStudents();

    std::ostringstream csv;
    // Header row
    csv << utils::CsvExporter::buildRow({
        "Student ID",
        "Roll Number",
        "Full Name",
        "Division",
        "Department",
        "Email",
        "Semester",
        "Status",
        "Registration Date"
    });

    for (const auto& s : students) {
        csv << utils::CsvExporter::buildRow({
            s.id,
            s.roll_number,
            s.full_name,
            s.division,
            s.department,
            s.email,
            std::to_string(s.semester),
            s.is_active ? "Active" : "Inactive / Archived",
            s.created_at
        });
    }

    return ServiceResult<std::string>::ok(csv.str(), 200);
}

} // namespace smartclass
