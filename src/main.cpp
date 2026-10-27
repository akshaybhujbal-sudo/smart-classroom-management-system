#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include "../include/storage/FileStorageManager.hpp"
#include "../include/services/StudentManager.hpp"
#include "../include/services/AttendanceManager.hpp"
#include "../include/services/TimetableManager.hpp"
#include "../include/services/ClassroomManager.hpp"
#include "../include/services/ReportService.hpp"
#include "../include/controllers/ApiController.hpp"

using namespace smartclass;

std::unique_ptr<httplib::Server> g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[SHUTDOWN] Intercepted signal (" << signum << "). Stopping server cleanly...\n";
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string host = "127.0.0.1";
    std::string data_dir = "data";
    std::string web_dir = "web";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--data" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--web" && i + 1 < argc) {
            web_dir = argv[++i];
        }
    }

    std::cout << "=================================================================\n";
    std::cout << "      SMART CLASSROOM MANAGEMENT SYSTEM — C++17 REST ENGINE      \n";
    std::cout << "=================================================================\n";

    // 1. Storage Layer
    auto storage = std::make_shared<FileStorageManager>(data_dir);
    std::cout << "[INIT] Data repository initialized at: '" << data_dir << "'\n";

    // 2. Business Service Layer
    auto student_mgr = std::make_shared<StudentManager>(storage);
    auto attendance_mgr = std::make_shared<AttendanceManager>(storage, student_mgr);
    auto timetable_mgr = std::make_shared<TimetableManager>(storage);
    auto classroom_mgr = std::make_shared<ClassroomManager>(storage);
    auto report_svc = std::make_shared<ReportService>(storage, student_mgr, attendance_mgr, timetable_mgr, classroom_mgr);

    std::cout << "[INIT] Business services initialized (Student, Attendance, Timetable, Classroom, Reports).\n";

    // 3. API & Web Routing Layer
    g_server = std::make_unique<httplib::Server>();
    ApiController api_ctrl(student_mgr, attendance_mgr, timetable_mgr, classroom_mgr, report_svc, web_dir);
    api_ctrl.registerRoutes(*g_server);

    std::cout << "[INIT] REST API routes registered on endpoints:\n";
    std::cout << "       - GET    /api/dashboard\n";
    std::cout << "       - CRUD   /api/students\n";
    std::cout << "       - CRUD   /api/attendance\n";
    std::cout << "       - CRUD   /api/timetable\n";
    std::cout << "       - GET/PUT /api/classroom\n";
    std::cout << "       - GET    /api/reports/...\n";

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "-----------------------------------------------------------------\n";
    std::cout << "  Server online at: http://" << host << ":" << port << "/\n";
    std::cout << "  Press Ctrl+C to terminate server.\n";
    std::cout << "-----------------------------------------------------------------\n";

    try {
        if (!g_server->listen(host.c_str(), port)) {
            std::cerr << "[ERROR] Could not bind server to " << host << ":" << port << "\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "[FATAL SERVER ERROR] " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "[FATAL SERVER ERROR] Unknown exception caught in main listener.\n";
        return 1;
    }

    std::cout << "[STOP] Server shutdown complete. Goodbye!\n";
    return 0;
}
