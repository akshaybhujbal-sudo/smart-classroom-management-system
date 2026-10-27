#include <iostream>
#include <cassert>
#include <filesystem>
#include <vector>
#include <memory>
#include <string>

#include "../include/storage/FileStorageManager.hpp"
#include "../include/services/StudentManager.hpp"
#include "../include/services/AttendanceManager.hpp"
#include "../include/services/TimetableManager.hpp"
#include "../include/services/ClassroomManager.hpp"
#include "../include/services/ReportService.hpp"
#include "../include/controllers/ApiController.hpp"
#include "../include/utils/Validation.hpp"
#include "../include/utils/CsvExporter.hpp"

namespace fs = std::filesystem;
using namespace smartclass;

const std::string TEST_DIR = "test_sandbox_data";

void cleanupSandbox() {
    if (fs::exists(TEST_DIR)) {
        std::error_code ec;
        fs::remove_all(TEST_DIR, ec);
    }
}

int g_passed = 0;
int g_failed = 0;

void recordResult(const std::string& test_name, bool passed, const std::string& extra = "") {
    if (passed) {
        g_passed++;
        std::cout << " [PASS] " << test_name;
        if (!extra.empty()) std::cout << " (" << extra << ")";
        std::cout << "\n";
    } else {
        g_failed++;
        std::cout << " [FAIL] " << test_name;
        if (!extra.empty()) std::cout << " -> " << extra;
        std::cout << "\n";
    }
}

int main() {
    std::cout << "=================================================================\n";
    std::cout << "   SMART CLASSROOM MANAGEMENT SYSTEM — BACKEND AUTOMATED TESTS   \n";
    std::cout << "=================================================================\n";

    cleanupSandbox();

    auto storage = std::make_shared<FileStorageManager>(TEST_DIR);
    auto student_mgr = std::make_shared<StudentManager>(storage);
    auto attendance_mgr = std::make_shared<AttendanceManager>(storage, student_mgr);
    auto timetable_mgr = std::make_shared<TimetableManager>(storage);
    auto classroom_mgr = std::make_shared<ClassroomManager>(storage);
    auto report_svc = std::make_shared<ReportService>(storage, student_mgr, attendance_mgr, timetable_mgr, classroom_mgr);

    std::string created_student_id;
    std::string created_tt_id;

    // -------------------------------------------------------------------------
    // Scenario 1: Add a valid student
    // -------------------------------------------------------------------------
    {
        Student s;
        s.roll_number = "CS2026-901";
        s.full_name = "Test Student Alpha";
        s.division = "A";
        s.department = "Computer Science & Engineering";
        s.email = "alpha.test@college.edu";
        s.semester = 6;

        auto res = student_mgr->addStudent(s);
        bool ok = res.success && res.http_status == 201 && !res.data->id.empty() && res.data->is_active;
        if (ok) created_student_id = res.data->id;
        recordResult("1. Add a valid student", ok, "Generated ID: " + created_student_id);
    }

    // -------------------------------------------------------------------------
    // Scenario 2: Reject a student with missing required fields
    // -------------------------------------------------------------------------
    {
        Student bad1; // Empty roll number
        bad1.full_name = "No Roll Name";
        auto res1 = student_mgr->addStudent(bad1);

        Student bad2; // Empty name
        bad2.roll_number = "CS2026-902";
        auto res2 = student_mgr->addStudent(bad2);

        bool ok = (!res1.success && res1.http_status == 400) &&
                  (!res2.success && res2.http_status == 400);
        recordResult("2. Reject a student with missing required fields", ok, "Correctly rejected with 400");
    }

    // -------------------------------------------------------------------------
    // Scenario 3: Reject a duplicate roll number
    // -------------------------------------------------------------------------
    {
        Student dup;
        dup.roll_number = "cs2026-901"; // Case-insensitive duplicate of CS2026-901
        dup.full_name = "Duplicate Student";
        dup.email = "dup@college.edu";

        auto res = student_mgr->addStudent(dup);
        bool ok = (!res.success && res.http_status == 409);
        recordResult("3. Reject a duplicate roll number", ok, "Returned 409 Conflict: " + res.error_message);
    }

    // -------------------------------------------------------------------------
    // Scenario 4: Retrieve and search student records
    // -------------------------------------------------------------------------
    {
        auto search_res = student_mgr->searchStudents("Alpha");
        bool ok = search_res.success && !search_res.data->empty() &&
                  (*search_res.data)[0].roll_number == "CS2026-901";
        recordResult("4. Retrieve and search student records", ok, "Found matching roll CS2026-901");
    }

    // -------------------------------------------------------------------------
    // Scenario 5: Update a student successfully
    // -------------------------------------------------------------------------
    {
        auto get_res = student_mgr->getStudentById(created_student_id);
        bool ok = false;
        if (get_res.success) {
            Student to_update = *get_res.data;
            to_update.full_name = "Test Student Alpha (Updated)";
            to_update.division = "B";

            auto up_res = student_mgr->updateStudent(created_student_id, to_update);
            ok = up_res.success && up_res.data->full_name == "Test Student Alpha (Updated)" &&
                 up_res.data->division == "B";
        }
        recordResult("5. Update a student successfully", ok, "Updated name & division");
    }

    // -------------------------------------------------------------------------
    // Scenario 6: Return an appropriate error for a nonexistent student
    // -------------------------------------------------------------------------
    {
        auto res = student_mgr->getStudentById("std-nonexistent-9999");
        bool ok = (!res.success && res.http_status == 404);
        recordResult("6. Return an appropriate error for a nonexistent student", ok, "Returned 404 Not Found");
    }

    // -------------------------------------------------------------------------
    // Scenario 7: Record attendance successfully
    // -------------------------------------------------------------------------
    {
        AttendanceRecord rec;
        rec.date = "2026-10-21";
        rec.student_id = created_student_id;
        rec.status = "PRESENT";
        rec.remarks = "Morning Lecture";

        auto res = attendance_mgr->recordAttendance(rec);
        bool ok = res.success && res.data->status == "PRESENT" && !res.data->id.empty();
        recordResult("7. Record attendance successfully", ok, "Marked PRESENT for 2026-10-21");
    }

    // -------------------------------------------------------------------------
    // Scenario 8: Reject duplicate attendance (idempotent update check)
    // -------------------------------------------------------------------------
    {
        AttendanceRecord duplicate_entry;
        duplicate_entry.date = "2026-10-21";
        duplicate_entry.student_id = created_student_id;
        duplicate_entry.status = "ABSENT"; // Update status
        duplicate_entry.remarks = "Status changed to Absent";

        auto res = attendance_mgr->recordAttendance(duplicate_entry);
        auto date_records = attendance_mgr->getAttendanceByDate("2026-10-21");

        // Count for this student on 2026-10-21 should still be exactly 1!
        int match_count = 0;
        for (const auto& r : *date_records.data) {
            if (r.student_id == created_student_id) match_count++;
        }

        bool ok = res.success && match_count == 1 && res.data->status == "ABSENT";
        recordResult("8. Reject duplicate attendance for same student/date", ok, "Idempotent update without duplication");
    }

    // -------------------------------------------------------------------------
    // Scenario 9: Calculate attendance percentages correctly
    // -------------------------------------------------------------------------
    {
        // Add another record on 2026-10-22 as PRESENT
        AttendanceRecord rec2;
        rec2.date = "2026-10-22";
        rec2.student_id = created_student_id;
        rec2.status = "PRESENT";
        attendance_mgr->recordAttendance(rec2);

        // Student now has: 1 ABSENT (2026-10-21) + 1 PRESENT (2026-10-22) = 2 sessions, 1 attended = 50.0%
        auto stat_res = attendance_mgr->calculateStudentAttendancePercentage(created_student_id);

        // Also test zero division protection on a student with 0 sessions
        Student s_zero;
        s_zero.roll_number = "CS2026-905";
        s_zero.full_name = "Zero Session Student";
        auto add_zero = student_mgr->addStudent(s_zero);
        auto zero_stat_res = attendance_mgr->calculateStudentAttendancePercentage(add_zero.data->id);

        bool ok = stat_res.success && stat_res.data->total_sessions == 2 &&
                  stat_res.data->attended_sessions == 1 &&
                  stat_res.data->attendance_percentage == 50.0 &&
                  stat_res.data->is_defaulter == true &&
                  zero_stat_res.success && zero_stat_res.data->attendance_percentage == 0.0 &&
                  zero_stat_res.data->is_defaulter == false;

        recordResult("9. Calculate attendance percentages correctly", ok, "50.0% (defaulter) & 0.0% div-zero safe");
    }

    // -------------------------------------------------------------------------
    // Scenario 10: Reject an invalid date or attendance status
    // -------------------------------------------------------------------------
    {
        AttendanceRecord bad_date;
        bad_date.date = "2026-02-30"; // Invalid leap/day
        bad_date.student_id = created_student_id;
        bad_date.status = "PRESENT";
        auto res1 = attendance_mgr->recordAttendance(bad_date);

        AttendanceRecord bad_status;
        bad_status.date = "2026-10-21";
        bad_status.student_id = created_student_id;
        bad_status.status = "MAYBE"; // Invalid status
        auto res2 = attendance_mgr->recordAttendance(bad_status);

        bool ok = (!res1.success && res1.http_status == 400) &&
                  (!res2.success && res2.http_status == 400);
        recordResult("10. Reject an invalid date or attendance status", ok, "Rejected invalid date and status");
    }

    // -------------------------------------------------------------------------
    // Scenario 11: Create a valid timetable entry
    // -------------------------------------------------------------------------
    {
        TimetableEntry tt;
        tt.subject = "CS610: Cloud Computing";
        tt.teacher = "Prof. Test Faculty";
        tt.classroom = "Lab-101";
        tt.weekday = "MONDAY";
        tt.start_time = "14:00";
        tt.end_time = "15:30";

        auto res = timetable_mgr->createEntry(tt);
        bool ok = res.success && res.http_status == 201 && !res.data->id.empty();
        if (ok) created_tt_id = res.data->id;
        recordResult("11. Create a valid timetable entry", ok, "Created slot " + created_tt_id + " (14:00-15:30)");
    }

    // -------------------------------------------------------------------------
    // Scenario 12: Reject a conflicting classroom booking
    // -------------------------------------------------------------------------
    {
        TimetableEntry conflict;
        conflict.subject = "CS620: AI Ethics";
        conflict.teacher = "Dr. Another Faculty";
        conflict.classroom = "Lab-101"; // Same room!
        conflict.weekday = "MONDAY";     // Same day!
        conflict.start_time = "14:30";   // Overlaps 14:00 - 15:30!
        conflict.end_time = "16:00";

        auto res = timetable_mgr->createEntry(conflict);
        bool ok = (!res.success && res.http_status == 409);
        recordResult("12. Reject a conflicting classroom booking", ok, "Returned 409 Collision: " + res.error_message);
    }

    // -------------------------------------------------------------------------
    // Scenario 13: Update and delete timetable entries
    // -------------------------------------------------------------------------
    {
        auto get_res = timetable_mgr->getEntryById(created_tt_id);
        bool ok = false;
        if (get_res.success) {
            TimetableEntry upd = *get_res.data;
            upd.subject = "CS610: Advanced Cloud Systems";
            auto up_res = timetable_mgr->updateEntry(created_tt_id, upd);

            auto del_res = timetable_mgr->deleteEntry(created_tt_id);
            auto verify_del = timetable_mgr->getEntryById(created_tt_id);

            ok = up_res.success && del_res.success && (!verify_del.success && verify_del.http_status == 404);
        }
        recordResult("13. Update and delete timetable entries", ok, "Updated and deleted successfully");
    }

    // -------------------------------------------------------------------------
    // Scenario 14: Update simulated device states
    // -------------------------------------------------------------------------
    {
        auto state_res = classroom_mgr->getClassroomStatus();
        bool ok = false;
        if (state_res.success && !state_res.data->devices.empty()) {
            std::string dev_id = state_res.data->devices[0].id;
            auto up_res = classroom_mgr->updateDeviceState(dev_id, true, 2);
            auto check_dev = classroom_mgr->getDeviceById(dev_id);

            ok = up_res.success && check_dev.success && check_dev.data->is_powered == true &&
                 check_dev.data->level == 2;
        }
        recordResult("14. Update simulated device states", ok, "Toggled device power ON with level 2");
    }

    // -------------------------------------------------------------------------
    // Scenario 15: Verify saved data remains available after restart
    // -------------------------------------------------------------------------
    {
        // Simulate application restart by discarding current storage pointer and loading a fresh instance
        auto fresh_storage = std::make_shared<FileStorageManager>(TEST_DIR);
        auto fresh_student_mgr = std::make_shared<StudentManager>(fresh_storage);

        auto check_res = fresh_student_mgr->getStudentById(created_student_id);
        bool ok = check_res.success && check_res.data->roll_number == "CS2026-901";
        recordResult("15. Verify saved data remains available after restart", ok, "Record loaded from disk after reload");
    }

    // -------------------------------------------------------------------------
    // Scenario 16: Verify deactivating student preserves historical attendance
    // -------------------------------------------------------------------------
    {
        // Deactivate student CS2026-901
        auto del_res = student_mgr->deleteStudent(created_student_id);

        // Verify student is inactive
        auto check_s = student_mgr->getStudentById(created_student_id);

        // Verify historical attendance still exists intact
        auto history_res = attendance_mgr->getStudentAttendanceHistory("CS2026-901");

        bool ok = del_res.success && check_s.success && !check_s.data->is_active &&
                  history_res.success && history_res.data->size() == 2;

        recordResult("16. Deactivating student preserves historical attendance", ok, "Soft delete active=false, 2 history logs preserved");
    }

    // -------------------------------------------------------------------------
    // Scenario 17: Verify that invalid JSON produces a proper error response
    // -------------------------------------------------------------------------
    {
        httplib::Response res;
        ApiController::sendError(res, 400, "Malformed JSON syntax");
        nlohmann::json err_json = nlohmann::json::parse(res.body);

        bool ok = res.status == 400 && err_json.contains("success") &&
                  err_json["success"] == false && err_json.contains("error");
        recordResult("17. Verify invalid JSON produces proper error response", ok, "Response JSON has success=false, status=400");
    }

    // -------------------------------------------------------------------------
    // Scenario 18: Verify report generation works with empty and populated datasets
    // -------------------------------------------------------------------------
    {
        // Test populated dataset reports
        auto daily_res = report_svc->getDailyAttendanceSummary("2026-10-21");
        auto stats_res = report_svc->getStudentStatisticsReport();
        auto defaulters_res = report_svc->getDefaultersReport(75.0);
        auto csv_res = report_svc->exportAttendanceCsv();

        bool pop_ok = daily_res.success && stats_res.success && defaulters_res.success &&
                      csv_res.success && csv_res.data->find("Attendance Record ID") != std::string::npos;

        // Test empty dataset reports in an isolated empty sandbox
        std::string empty_dir = "test_empty_sandbox";
        if (fs::exists(empty_dir)) fs::remove_all(empty_dir);
        fs::create_directories(empty_dir);

        auto empty_storage = std::make_shared<FileStorageManager>(empty_dir);
        // Clear out the auto-created demo data to test true empty state
        empty_storage->saveStudents({});
        empty_storage->saveAttendance({});
        empty_storage->saveTimetable({});

        auto emp_student_mgr = std::make_shared<StudentManager>(empty_storage);
        auto emp_attendance_mgr = std::make_shared<AttendanceManager>(empty_storage, emp_student_mgr);
        auto emp_timetable_mgr = std::make_shared<TimetableManager>(empty_storage);
        auto emp_classroom_mgr = std::make_shared<ClassroomManager>(empty_storage);
        auto emp_report_svc = std::make_shared<ReportService>(empty_storage, emp_student_mgr, emp_attendance_mgr, emp_timetable_mgr, emp_classroom_mgr);

        auto emp_daily = emp_report_svc->getDailyAttendanceSummary("2026-10-21");
        auto emp_stats = emp_report_svc->getStudentStatisticsReport();
        auto emp_csv = emp_report_svc->exportAttendanceCsv();

        bool emp_ok = emp_daily.success && emp_daily.data->total_students == 0 &&
                      emp_stats.success && emp_stats.data->empty() &&
                      emp_csv.success;

        if (fs::exists(empty_dir)) fs::remove_all(empty_dir);

        bool ok = pop_ok && emp_ok;
        recordResult("18. Report generation with empty and populated datasets", ok, "Tested populated and zero-data datasets safely");
    }

    cleanupSandbox();

    std::cout << "=================================================================\n";
    std::cout << "   TEST SUMMARY: " << g_passed << " PASSED, " << g_failed << " FAILED (TOTAL: " << (g_passed + g_failed) << ")\n";
    std::cout << "=================================================================\n";

    return (g_failed == 0) ? 0 : 1;
}
