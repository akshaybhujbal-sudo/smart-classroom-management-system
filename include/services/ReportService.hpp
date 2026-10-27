#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../third_party/nlohmann/json.hpp"
#include "../services/StudentManager.hpp"
#include "../services/AttendanceManager.hpp"
#include "../services/TimetableManager.hpp"
#include "../services/ClassroomManager.hpp"
#include "../storage/FileStorageManager.hpp"
#include "../utils/ServiceResult.hpp"

namespace smartclass {

class ReportService {
public:
    ReportService(std::shared_ptr<FileStorageManager> storage,
                  std::shared_ptr<StudentManager> student_mgr,
                  std::shared_ptr<AttendanceManager> attendance_mgr,
                  std::shared_ptr<TimetableManager> timetable_mgr,
                  std::shared_ptr<ClassroomManager> classroom_mgr);

    // Analytical reports
    ServiceResult<DailyAttendanceSummary> getDailyAttendanceSummary(const std::string& date);
    ServiceResult<std::vector<StudentAttendanceStat>> getStudentStatisticsReport();
    ServiceResult<std::vector<StudentAttendanceStat>> getDefaultersReport(double threshold = 75.0);
    ServiceResult<nlohmann::json> getClassroomStatusSummary();
    ServiceResult<nlohmann::json> getDashboardSummary();

    // CSV Generation (in-memory string stream to prevent filesystem path vulnerabilities)
    ServiceResult<std::string> exportAttendanceCsv(const std::string& date_filter = "");
    ServiceResult<std::string> exportStudentsCsv();

private:
    std::shared_ptr<FileStorageManager> storage_;
    std::shared_ptr<StudentManager> student_mgr_;
    std::shared_ptr<AttendanceManager> attendance_mgr_;
    std::shared_ptr<TimetableManager> timetable_mgr_;
    std::shared_ptr<ClassroomManager> classroom_mgr_;
};

} // namespace smartclass
