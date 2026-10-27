#pragma once

#include <memory>
#include <string>
#include "../third_party/httplib.h"
#include "../third_party/nlohmann/json.hpp"
#include "../services/StudentManager.hpp"
#include "../services/AttendanceManager.hpp"
#include "../services/TimetableManager.hpp"
#include "../services/ClassroomManager.hpp"
#include "../services/ReportService.hpp"

namespace smartclass {

class ApiController {
public:
    ApiController(std::shared_ptr<StudentManager> student_mgr,
                  std::shared_ptr<AttendanceManager> attendance_mgr,
                  std::shared_ptr<TimetableManager> timetable_mgr,
                  std::shared_ptr<ClassroomManager> classroom_mgr,
                  std::shared_ptr<ReportService> report_svc,
                  std::string web_dir = "web");

    // Route Registration
    void registerRoutes(httplib::Server& server);

    // Helpers
    static void sendJson(httplib::Response& res, int status, const nlohmann::json& payload);
    static void sendError(httplib::Response& res, int status, const std::string& message);
    static void sendSuccess(httplib::Response& res, int status, const nlohmann::json& data, const std::string& message = "");

private:
    std::shared_ptr<StudentManager> student_mgr_;
    std::shared_ptr<AttendanceManager> attendance_mgr_;
    std::shared_ptr<TimetableManager> timetable_mgr_;
    std::shared_ptr<ClassroomManager> classroom_mgr_;
    std::shared_ptr<ReportService> report_svc_;
    std::string web_dir_;

    void registerDashboardRoutes(httplib::Server& server);
    void registerStudentRoutes(httplib::Server& server);
    void registerAttendanceRoutes(httplib::Server& server);
    void registerTimetableRoutes(httplib::Server& server);
    void registerClassroomRoutes(httplib::Server& server);
    void registerReportRoutes(httplib::Server& server);
    void registerStaticRoutes(httplib::Server& server);
};

} // namespace smartclass
