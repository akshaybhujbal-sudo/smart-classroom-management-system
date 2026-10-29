import os
import sys
import shutil
import unittest
import tempfile

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, BASE_DIR)

from backend_python.storage.file_storage_manager import FileStorageManager
from backend_python.services.student_manager import StudentManager
from backend_python.services.attendance_manager import AttendanceManager
from backend_python.services.timetable_manager import TimetableManager
from backend_python.services.classroom_manager import ClassroomManager
from backend_python.services.report_service import ReportService
from backend_python.app import create_app

class BackendPythonTests(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp()
        self.storage = FileStorageManager(data_dir=self.test_dir)
        self.student_mgr = StudentManager(self.storage)
        self.attendance_mgr = AttendanceManager(self.storage, self.student_mgr)
        self.timetable_mgr = TimetableManager(self.storage)
        self.classroom_mgr = ClassroomManager(self.storage)
        self.report_svc = ReportService(self.storage, self.student_mgr, self.attendance_mgr, self.timetable_mgr, self.classroom_mgr)

    def tearDown(self):
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_01_add_valid_student(self):
        res = self.student_mgr.add_student({
            "roll_number": "CS2026-901",
            "full_name": "Test Student 1",
            "division": "A",
            "department": "CSE",
            "semester": 6
        })
        self.assertTrue(res["success"])
        self.assertEqual(res["status"], 201)
        self.assertEqual(res["data"]["id"], "std-001")

    def test_02_reject_student_missing_fields(self):
        res = self.student_mgr.add_student({"roll_number": ""})
        self.assertFalse(res["success"])
        self.assertEqual(res["status"], 400)

    def test_03_reject_duplicate_roll_number(self):
        self.student_mgr.add_student({"roll_number": "CS2026-901", "full_name": "Student A"})
        res = self.student_mgr.add_student({"roll_number": "cs2026-901", "full_name": "Student B"})
        self.assertFalse(res["success"])
        self.assertEqual(res["status"], 409)

    def test_04_search_students(self):
        self.student_mgr.add_student({"roll_number": "CS2026-901", "full_name": "Arjun Sharma"})
        res = self.student_mgr.search_students("arjun")
        self.assertTrue(res["success"])
        self.assertEqual(len(res["data"]), 1)

    def test_05_update_student(self):
        add_res = self.student_mgr.add_student({"roll_number": "CS2026-901", "full_name": "Old Name"})
        sid = add_res["data"]["id"]
        res = self.student_mgr.update_student(sid, {"full_name": "New Name", "division": "B"})
        self.assertTrue(res["success"])
        self.assertEqual(res["data"]["full_name"], "New Name")
        self.assertEqual(res["data"]["division"], "B")

    def test_06_nonexistent_student_returns_404(self):
        res = self.student_mgr.get_student_by_id("non-existent-id")
        self.assertFalse(res["success"])
        self.assertEqual(res["status"], 404)

    def test_07_record_attendance(self):
        res = self.attendance_mgr.record_attendance({
            "student_id": "std-001",
            "date": "2026-10-21",
            "status": "PRESENT"
        })
        self.assertTrue(res["success"])
        self.assertEqual(res["data"]["status"], "PRESENT")

    def test_08_idempotent_duplicate_attendance(self):
        self.attendance_mgr.record_attendance({"student_id": "std-001", "date": "2026-10-21", "status": "PRESENT"})
        res2 = self.attendance_mgr.record_attendance({"student_id": "std-001", "date": "2026-10-21", "status": "ABSENT"})
        self.assertTrue(res2["success"])
        all_recs = self.attendance_mgr._get_all_raw()
        self.assertEqual(len(all_recs), 1)
        self.assertEqual(all_recs[0]["status"], "ABSENT")

    def test_09_attendance_percentage_and_div_zero(self):
        self.attendance_mgr.record_attendance({"student_id": "std-001", "date": "2026-10-20", "status": "PRESENT"})
        self.attendance_mgr.record_attendance({"student_id": "std-001", "date": "2026-10-21", "status": "ABSENT"})
        hist = self.attendance_mgr.get_student_attendance_history("std-001")
        self.assertEqual(hist["data"]["percentage"], 50.0)

        empty_hist = self.attendance_mgr.get_student_attendance_history("std-999")
        self.assertEqual(empty_hist["data"]["percentage"], 0.0)

    def test_10_reject_invalid_date_or_status(self):
        res_date = self.attendance_mgr.record_attendance({"student_id": "std-001", "date": "invalid-date", "status": "PRESENT"})
        self.assertEqual(res_date["status"], 400)
        res_status = self.attendance_mgr.record_attendance({"student_id": "std-001", "date": "2026-10-21", "status": "MAYBE"})
        self.assertEqual(res_status["status"], 400)

    def test_11_create_valid_timetable_entry(self):
        res = self.timetable_mgr.create_entry({
            "subject": "CS601: ML",
            "teacher": "Dr. Smith",
            "classroom": "Lab-101",
            "weekday": "MONDAY",
            "start_time": "14:00",
            "end_time": "15:30"
        })
        self.assertTrue(res["success"])
        self.assertEqual(res["status"], 201)

    def test_12_timetable_collision_detection(self):
        self.timetable_mgr.create_entry({
            "subject": "Cloud Computing",
            "teacher": "Prof. Mehta",
            "classroom": "Lab-101",
            "weekday": "MONDAY",
            "start_time": "14:00",
            "end_time": "15:30"
        })
        # Conflict same room same time
        res = self.timetable_mgr.create_entry({
            "subject": "AI Lab",
            "teacher": "Prof. Rao",
            "classroom": "Lab-101",
            "weekday": "MONDAY",
            "start_time": "14:30",
            "end_time": "16:00"
        })
        self.assertFalse(res["success"])
        self.assertEqual(res["status"], 409)

    def test_13_update_and_delete_timetable(self):
        c_res = self.timetable_mgr.create_entry({
            "subject": "Math", "teacher": "Prof. X", "classroom": "CR-1",
            "weekday": "TUESDAY", "start_time": "09:00", "end_time": "10:00"
        })
        tid = c_res["data"]["id"]
        up_res = self.timetable_mgr.update_entry(tid, {"subject": "Advanced Math"})
        self.assertTrue(up_res["success"])
        self.assertEqual(up_res["data"]["subject"], "Advanced Math")

        del_res = self.timetable_mgr.delete_entry(tid)
        self.assertTrue(del_res["success"])

    def test_14_update_simulated_devices(self):
        res = self.classroom_mgr.update_device_state("dev-001", True, level=3)
        self.assertTrue(res["success"])
        self.assertTrue(res["data"]["is_powered"])
        self.assertEqual(res["data"]["level"], 3)

    def test_15_persistence_across_reloads(self):
        self.student_mgr.add_student({"roll_number": "CS-999", "full_name": "Persistent Student"})
        # Instantiate brand new manager instance pointing to same storage directory
        fresh_storage = FileStorageManager(data_dir=self.test_dir)
        fresh_student_mgr = StudentManager(fresh_storage)
        students = fresh_student_mgr.get_all_students()["data"]
        self.assertEqual(len(students), 1)
        self.assertEqual(students[0]["roll_number"], "CS-999")

    def test_16_soft_delete_preserves_attendance(self):
        s_res = self.student_mgr.add_student({"roll_number": "CS-101", "full_name": "Alice"})
        sid = s_res["data"]["id"]
        self.attendance_mgr.record_attendance({"student_id": sid, "date": "2026-10-20", "status": "PRESENT"})
        self.attendance_mgr.record_attendance({"student_id": sid, "date": "2026-10-21", "status": "ABSENT"})

        del_res = self.student_mgr.delete_student(sid)
        self.assertTrue(del_res["success"])

        # Student is marked inactive
        student = self.student_mgr.get_student_by_id(sid)["data"]
        self.assertFalse(student["is_active"])

        # History is intact
        hist = self.attendance_mgr.get_student_attendance_history(sid)["data"]
        self.assertEqual(hist["total_classes"], 2)

    def test_17_flask_api_error_handling(self):
        app = create_app(data_dir=self.test_dir)
        client = app.test_client()

        # Nonexistent student returns 404
        r404 = client.get("/api/students/std-non-existent")
        self.assertEqual(r404.status_code, 404)
        data = r404.get_json()
        self.assertFalse(data["success"])

    def test_18_report_generation(self):
        report = self.report_svc.get_dashboard_summary()
        self.assertTrue(report["success"])
        self.assertIn("attendance_trend", report["data"])
        self.assertIn("students", report["data"])

def run_tests():
    suite = unittest.TestLoader().loadTestsFromTestCase(BackendPythonTests)
    print("=" * 65)
    print("   SMART CLASSROOM MANAGEMENT SYSTEM — PYTHON TEST SUITE   ")
    print("=" * 65)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    print("=" * 65)
    print(f"   TEST SUMMARY: {result.testsRun - len(result.failures) - len(result.errors)} PASSED, {len(result.failures) + len(result.errors)} FAILED (TOTAL: {result.testsRun})")
    print("=" * 65)
    return len(result.failures) == 0 and len(result.errors) == 0

if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
