#include "../../include/controllers/ApiController.hpp"
#include "../../include/utils/Validation.hpp"
#include <iostream>
#include <filesystem>

namespace smartclass {

ApiController::ApiController(std::shared_ptr<StudentManager> student_mgr,
                             std::shared_ptr<AttendanceManager> attendance_mgr,
                             std::shared_ptr<TimetableManager> timetable_mgr,
                             std::shared_ptr<ClassroomManager> classroom_mgr,
                             std::shared_ptr<ReportService> report_svc,
                             std::string web_dir)
    : student_mgr_(std::move(student_mgr)),
      attendance_mgr_(std::move(attendance_mgr)),
      timetable_mgr_(std::move(timetable_mgr)),
      classroom_mgr_(std::move(classroom_mgr)),
      report_svc_(std::move(report_svc)),
      web_dir_(std::move(web_dir)) {}

void ApiController::sendJson(httplib::Response& res, int status, const nlohmann::json& payload) {
    res.status = status;
    res.set_content(payload.dump(2), "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

void ApiController::sendError(httplib::Response& res, int status, const std::string& message) {
    nlohmann::json payload = {
        {"success", false},
        {"status", status},
        {"error", message}
    };
    sendJson(res, status, payload);
}

void ApiController::sendSuccess(httplib::Response& res, int status, const nlohmann::json& data, const std::string& message) {
    nlohmann::json payload = {
        {"success", true},
        {"status", status},
        {"data", data}
    };
    if (!message.empty()) {
        payload["message"] = message;
    }
    sendJson(res, status, payload);
}

void ApiController::registerRoutes(httplib::Server& server) {
    // Global Exception Guard for HTTP worker threads
    server.set_exception_handler([](const auto&, auto& res, std::exception_ptr ep) {
        try {
            if (ep) std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            ApiController::sendError(res, 500, std::string("Internal Exception: ") + e.what());
        } catch (...) {
            ApiController::sendError(res, 500, "Unknown Internal Server Error");
        }
    });

    // CORS Preflight handler for all routes
    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    });

    registerDashboardRoutes(server);
    registerStudentRoutes(server);
    registerAttendanceRoutes(server);
    registerTimetableRoutes(server);
    registerClassroomRoutes(server);
    registerReportRoutes(server);
    registerStaticRoutes(server);
}

// -----------------------------------------------------------------------------
// Dashboard Routes
// -----------------------------------------------------------------------------
void ApiController::registerDashboardRoutes(httplib::Server& server) {
    server.Get("/api/dashboard", [this](const httplib::Request&, httplib::Response& res) {
        auto result = report_svc_->getDashboardSummary();
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });
}

// -----------------------------------------------------------------------------
// Student Routes
// -----------------------------------------------------------------------------
void ApiController::registerStudentRoutes(httplib::Server& server) {
    // GET /api/students (with optional ?search=... or ?include_inactive=...)
    server.Get("/api/students", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("search")) {
            std::string q = req.get_param_value("search");
            auto result = student_mgr_->searchStudents(q);
            sendSuccess(res, 200, *result.data);
            return;
        }

        bool include_inactive = true;
        if (req.has_param("include_inactive")) {
            std::string inc = req.get_param_value("include_inactive");
            if (inc == "false" || inc == "0") include_inactive = false;
        }

        auto result = student_mgr_->getAllStudents(include_inactive);
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // GET /api/students/:id
    server.Get(R"(/api/students/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto result = student_mgr_->getStudentById(id);
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // POST /api/students
    server.Post("/api/students", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload in request body.");
            return;
        }

        Student student;
        try {
            student = body.get<Student>();
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("JSON schema error: ") + e.what());
            return;
        }

        auto result = student_mgr_->addStudent(student);
        if (result.success) {
            sendSuccess(res, 201, *result.data, "Student registered successfully.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // PUT /api/students/:id
    server.Put(R"(/api/students/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload in request body.");
            return;
        }

        Student student;
        try {
            student = body.get<Student>();
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("JSON schema error: ") + e.what());
            return;
        }

        auto result = student_mgr_->updateStudent(id, student);
        if (result.success) {
            sendSuccess(res, 200, *result.data, "Student updated successfully.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // DELETE /api/students/:id (Soft Deletion)
    server.Delete(R"(/api/students/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto result = student_mgr_->deleteStudent(id);
        if (result.success) {
            sendSuccess(res, 200, {{"id", id}, {"status", "deactivated"}}, "Student record deactivated (historical data preserved).");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });
}

// -----------------------------------------------------------------------------
// Attendance Routes
// -----------------------------------------------------------------------------
void ApiController::registerAttendanceRoutes(httplib::Server& server) {
    // GET /api/attendance (by date: ?date=YYYY-MM-DD)
    server.Get("/api/attendance", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("date")) {
            std::string date = req.get_param_value("date");
            auto result = attendance_mgr_->getAttendanceByDate(date);
            if (result.success) {
                sendSuccess(res, 200, *result.data);
            } else {
                sendError(res, result.http_status, result.error_message);
            }
            return;
        }
        if (req.has_param("student")) {
            std::string std_id = req.get_param_value("student");
            auto result = attendance_mgr_->getStudentAttendanceHistory(std_id);
            if (result.success) {
                sendSuccess(res, 200, *result.data);
            } else {
                sendError(res, result.http_status, result.error_message);
            }
            return;
        }

        // Default: return today's attendance
        std::string today = utils::getCurrentDate();
        auto result = attendance_mgr_->getAttendanceByDate(today);
        sendSuccess(res, 200, *result.data);
    });

    // GET /api/attendance/student/:id
    server.Get(R"(/api/attendance/student/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto result = attendance_mgr_->getStudentAttendanceHistory(id);
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // POST /api/attendance
    server.Post("/api/attendance", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload.");
            return;
        }

        AttendanceRecord record;
        try {
            record = body.get<AttendanceRecord>();
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("JSON schema error: ") + e.what());
            return;
        }

        auto result = attendance_mgr_->recordAttendance(record);
        if (result.success) {
            sendSuccess(res, 200, *result.data, "Attendance marked successfully.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // PUT /api/attendance/:id
    server.Put(R"(/api/attendance/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload.");
            return;
        }

        std::string status = body.value("status", "");
        std::string remarks = body.value("remarks", "");

        auto result = attendance_mgr_->updateAttendance(id, status, remarks);
        if (result.success) {
            sendSuccess(res, 200, *result.data, "Attendance record updated.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // GET /api/reports/attendance?date=YYYY-MM-DD
    server.Get("/api/reports/attendance", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("date")) {
            std::string date = req.get_param_value("date");
            auto result = attendance_mgr_->getDailyAttendanceSummary(date);
            if (result.success) {
                sendSuccess(res, 200, *result.data);
            } else {
                sendError(res, result.http_status, result.error_message);
            }
            return;
        }

        auto result = report_svc_->getStudentStatisticsReport();
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });
}

// -----------------------------------------------------------------------------
// Timetable Routes
// -----------------------------------------------------------------------------
void ApiController::registerTimetableRoutes(httplib::Server& server) {
    // GET /api/timetable (with optional ?day=MONDAY)
    server.Get("/api/timetable", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("day")) {
            std::string day = req.get_param_value("day");
            auto result = timetable_mgr_->getEntriesByWeekday(day);
            if (result.success) {
                sendSuccess(res, 200, *result.data);
            } else {
                sendError(res, result.http_status, result.error_message);
            }
            return;
        }

        auto result = timetable_mgr_->getAllEntries();
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // POST /api/timetable
    server.Post("/api/timetable", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload.");
            return;
        }

        TimetableEntry entry;
        try {
            entry = body.get<TimetableEntry>();
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("JSON schema error: ") + e.what());
            return;
        }

        auto result = timetable_mgr_->createEntry(entry);
        if (result.success) {
            sendSuccess(res, 201, *result.data, "Timetable slot scheduled successfully.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // PUT /api/timetable/:id
    server.Put(R"(/api/timetable/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload.");
            return;
        }

        TimetableEntry entry;
        try {
            entry = body.get<TimetableEntry>();
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("JSON schema error: ") + e.what());
            return;
        }

        auto result = timetable_mgr_->updateEntry(id, entry);
        if (result.success) {
            sendSuccess(res, 200, *result.data, "Timetable slot updated successfully.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // DELETE /api/timetable/:id
    server.Delete(R"(/api/timetable/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto result = timetable_mgr_->deleteEntry(id);
        if (result.success) {
            sendSuccess(res, 200, {{"id", id}, {"status", "deleted"}}, "Timetable slot removed.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });
}

// -----------------------------------------------------------------------------
// Classroom IoT Routes
// -----------------------------------------------------------------------------
void ApiController::registerClassroomRoutes(httplib::Server& server) {
    // GET /api/classroom
    server.Get("/api/classroom", [this](const httplib::Request&, httplib::Response& res) {
        auto result = classroom_mgr_->getClassroomStatus();
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // PUT and POST /api/classroom/devices
    auto handleDeviceUpdate = [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Malformed or invalid JSON payload.");
            return;
        }

        std::string device_id = body.value("device_id", "");
        if (device_id.empty()) device_id = body.value("id", "");

        bool is_powered = body.value("is_powered", false);
        int level = body.value("level", 0);

        auto result = classroom_mgr_->updateDeviceState(device_id, is_powered, level);
        if (result.success) {
            sendSuccess(res, 200, *result.data, "Device state updated successfully.");
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    };

    server.Put("/api/classroom/devices", handleDeviceUpdate);
    server.Post("/api/classroom/devices", handleDeviceUpdate);
}

// -----------------------------------------------------------------------------
// Report Routes
// -----------------------------------------------------------------------------
void ApiController::registerReportRoutes(httplib::Server& server) {
    // GET /api/reports/students
    server.Get("/api/reports/students", [this](const httplib::Request&, httplib::Response& res) {
        auto result = report_svc_->getStudentStatisticsReport();
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // GET /api/reports/defaulters
    server.Get("/api/reports/defaulters", [this](const httplib::Request&, httplib::Response& res) {
        auto result = report_svc_->getDefaultersReport(75.0);
        if (result.success) {
            sendSuccess(res, 200, *result.data);
        } else {
            sendError(res, result.http_status, result.error_message);
        }
    });

    // GET /api/reports/export-csv
    server.Get("/api/reports/export-csv", [this](const httplib::Request& req, httplib::Response& res) {
        std::string type = "attendance";
        if (req.has_param("type")) {
            type = req.get_param_value("type");
        }

        if (type == "students") {
            auto result = report_svc_->exportStudentsCsv();
            res.status = 200;
            res.set_content(*result.data, "text/csv");
            res.set_header("Content-Disposition", "attachment; filename=\"students_report.csv\"");
            res.set_header("Access-Control-Allow-Origin", "*");
        } else {
            std::string date_filter = "";
            if (req.has_param("date")) {
                date_filter = req.get_param_value("date");
            }
            auto result = report_svc_->exportAttendanceCsv(date_filter);
            res.status = 200;
            res.set_content(*result.data, "text/csv");
            res.set_header("Content-Disposition", "attachment; filename=\"attendance_report.csv\"");
            res.set_header("Access-Control-Allow-Origin", "*");
        }
    });
}

// -----------------------------------------------------------------------------
// Static Web UI Routes
// -----------------------------------------------------------------------------
void ApiController::registerStaticRoutes(httplib::Server& server) {
    if (std::filesystem::exists(web_dir_)) {
        server.set_mount_point("/", web_dir_);

        // Explicit fallback for root index.html to guarantee direct browser access
        server.Get("/", [this](const httplib::Request&, httplib::Response& res) {
            std::filesystem::path index_path = std::filesystem::path(web_dir_) / "index.html";
            if (std::filesystem::exists(index_path)) {
                std::ifstream ifs(index_path, std::ios::in | std::ios::binary);
                std::ostringstream ss;
                ss << ifs.rdbuf();
                res.set_content(ss.str(), "text/html");
            } else {
                res.status = 404;
                res.set_content("<h1>404 Not Found: index.html missing in web directory</h1>", "text/html");
            }
        });
    }
}

} // namespace smartclass
