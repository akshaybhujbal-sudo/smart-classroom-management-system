import os
import sys
import argparse
from flask import Flask, request, jsonify, send_from_directory, Response

# Add current directory to path
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, BASE_DIR)

from backend_python.storage.file_storage_manager import FileStorageManager
from backend_python.services.student_manager import StudentManager
from backend_python.services.attendance_manager import AttendanceManager
from backend_python.services.timetable_manager import TimetableManager
from backend_python.services.classroom_manager import ClassroomManager
from backend_python.services.report_service import ReportService

def create_app(data_dir=None, web_dir=None):
    if data_dir is None:
        data_dir = os.path.join(BASE_DIR, "data")
    if web_dir is None:
        web_dir = os.path.join(BASE_DIR, "web")

    app = Flask(__name__, static_folder=None)

    # Initialize domain services
    storage = FileStorageManager(data_dir=data_dir)
    student_mgr = StudentManager(storage)
    attendance_mgr = AttendanceManager(storage, student_mgr)
    timetable_mgr = TimetableManager(storage)
    classroom_mgr = ClassroomManager(storage)
    report_svc = ReportService(storage, student_mgr, attendance_mgr, timetable_mgr, classroom_mgr)

    # Add CORS headers to all responses
    @app.after_request
    def add_cors_headers(response):
        response.headers["Access-Control-Allow-Origin"] = "*"
        response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS"
        response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization"
        return response

    # Global OPTIONS preflight handler
    @app.route("/<path:dummy>", methods=["OPTIONS"])
    @app.route("/", methods=["OPTIONS"])
    def options_handler(dummy=""):
        return Response("", status=204)

    # ---------------------------------------------------------
    # Dashboard API
    # ---------------------------------------------------------
    @app.route("/api/dashboard", methods=["GET"])
    def get_dashboard():
        res = report_svc.get_dashboard_summary()
        return jsonify(res), res.get("status", 200)

    # ---------------------------------------------------------
    # Student API
    # ---------------------------------------------------------
    @app.route("/api/students", methods=["GET"])
    def get_students():
        search_query = request.args.get("search")
        if search_query is not None:
            res = student_mgr.search_students(search_query)
            return jsonify(res), res.get("status", 200)

        include_inactive = request.args.get("include_inactive", "true").lower() not in ("false", "0")
        res = student_mgr.get_all_students(include_inactive=include_inactive)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/students/<student_id>", methods=["GET"])
    def get_student(student_id):
        res = student_mgr.get_student_by_id(student_id)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/students", methods=["POST"])
    def add_student():
        body = request.get_json(silent=True) or {}
        res = student_mgr.add_student(body)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/students/<student_id>", methods=["PUT"])
    def update_student(student_id):
        body = request.get_json(silent=True) or {}
        res = student_mgr.update_student(student_id, body)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/students/<student_id>", methods=["DELETE"])
    def delete_student(student_id):
        res = student_mgr.delete_student(student_id)
        return jsonify(res), res.get("status", 200)

    # ---------------------------------------------------------
    # Attendance API
    # ---------------------------------------------------------
    @app.route("/api/attendance", methods=["GET"])
    def get_attendance():
        date_param = request.args.get("date")
        student_param = request.args.get("student")

        if date_param:
            res = attendance_mgr.get_attendance_by_date(date_param)
            return jsonify(res), res.get("status", 200)

        if student_param:
            res = attendance_mgr.get_student_attendance_history(student_param)
            return jsonify(res), res.get("status", 200)

        from datetime import datetime
        today_str = datetime.utcnow().strftime("%Y-%m-%d")
        res = attendance_mgr.get_attendance_by_date(today_str)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/attendance/student/<student_id>", methods=["GET"])
    def get_student_attendance(student_id):
        res = attendance_mgr.get_student_attendance_history(student_id)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/attendance", methods=["POST"])
    def record_attendance():
        body = request.get_json(silent=True) or {}
        res = attendance_mgr.record_attendance(body)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/attendance/<record_id>", methods=["PUT"])
    def update_attendance(record_id):
        body = request.get_json(silent=True) or {}
        status = body.get("status", "")
        remarks = body.get("remarks")
        res = attendance_mgr.update_attendance(record_id, status, remarks)
        return jsonify(res), res.get("status", 200)

    # ---------------------------------------------------------
    # Timetable API
    # ---------------------------------------------------------
    @app.route("/api/timetable", methods=["GET"])
    def get_timetable():
        day_param = request.args.get("day")
        if day_param:
            res = timetable_mgr.get_entries_by_weekday(day_param)
            return jsonify(res), res.get("status", 200)
        res = timetable_mgr.get_all_entries()
        return jsonify(res), res.get("status", 200)

    @app.route("/api/timetable", methods=["POST"])
    def create_timetable():
        body = request.get_json(silent=True) or {}
        res = timetable_mgr.create_entry(body)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/timetable/<entry_id>", methods=["PUT"])
    def update_timetable(entry_id):
        body = request.get_json(silent=True) or {}
        res = timetable_mgr.update_entry(entry_id, body)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/timetable/<entry_id>", methods=["DELETE"])
    def delete_timetable(entry_id):
        res = timetable_mgr.delete_entry(entry_id)
        return jsonify(res), res.get("status", 200)

    # ---------------------------------------------------------
    # Classroom IoT API
    # ---------------------------------------------------------
    @app.route("/api/classroom", methods=["GET"])
    def get_classroom():
        res = classroom_mgr.get_classroom_status()
        return jsonify(res), res.get("status", 200)

    @app.route("/api/classroom/devices", methods=["PUT", "POST"])
    def update_device():
        body = request.get_json(silent=True) or {}
        device_id = body.get("device_id") or body.get("id", "")
        is_powered = body.get("is_powered", False)
        level = body.get("level")
        res = classroom_mgr.update_device_state(device_id, is_powered, level)
        return jsonify(res), res.get("status", 200)

    # ---------------------------------------------------------
    # Reports API
    # ---------------------------------------------------------
    @app.route("/api/reports/students", methods=["GET"])
    def report_students():
        res = report_svc.get_student_statistics_report()
        return jsonify(res), res.get("status", 200)

    @app.route("/api/reports/defaulters", methods=["GET"])
    def report_defaulters():
        res = report_svc.get_defaulters_report(75.0)
        return jsonify(res), res.get("status", 200)

    @app.route("/api/reports/attendance", methods=["GET"])
    def report_attendance():
        date_param = request.args.get("date")
        if date_param:
            res = attendance_mgr.get_daily_attendance_summary(date_param)
            return jsonify(res), res.get("status", 200)
        res = report_svc.get_student_statistics_report()
        return jsonify(res), res.get("status", 200)

    @app.route("/api/reports/export-csv", methods=["GET"])
    def export_csv():
        export_type = request.args.get("type", "attendance")
        if export_type == "students":
            csv_content = report_svc.export_students_csv()
            filename = "students_report.csv"
        else:
            date_filter = request.args.get("date", "")
            csv_content = report_svc.export_attendance_csv(date_filter)
            filename = "attendance_report.csv"

        return Response(
            csv_content,
            mimetype="text/csv",
            headers={"Content-Disposition": f'attachment; filename="{filename}"'}
        )

    # ---------------------------------------------------------
    # Static Web UI Hosting
    # ---------------------------------------------------------
    @app.route("/", methods=["GET"])
    def serve_index():
        return send_from_directory(web_dir, "index.html")

    @app.route("/<path:filename>", methods=["GET"])
    def serve_static(filename):
        target = os.path.join(web_dir, filename)
        if os.path.exists(target):
            return send_from_directory(web_dir, filename)
        # Fallback to index.html for SPA routes
        return send_from_directory(web_dir, "index.html")

    return app

def main():
    parser = argparse.ArgumentParser(description="Smart Classroom Management System - Python Backend")
    parser.add_argument("--port", type=int, default=8080, help="Port to listen on (default 8080)")
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Host interface (default 127.0.0.1)")
    parser.add_argument("--data", type=str, default="data", help="Directory for JSON persistence")
    parser.add_argument("--web", type=str, default="web", help="Directory for static web frontend")

    args = parser.parse_args()

    data_path = os.path.abspath(args.data)
    web_path = os.path.abspath(args.web)

    print("=" * 65)
    print("   SMART CLASSROOM MANAGEMENT SYSTEM — PYTHON FLASK ENGINE   ")
    print("=" * 65)
    print(f"[INIT] Data repository at: '{data_path}'")
    print(f"[INIT] Web directory at:    '{web_path}'")

    app = create_app(data_dir=data_path, web_dir=web_path)

    print("-----------------------------------------------------------------")
    print(f"  Server online at: http://{args.host}:{args.port}/")
    print("  Press Ctrl+C to terminate server.")
    print("-----------------------------------------------------------------")

    app.run(host=args.host, port=args.port, debug=False, threaded=True)

if __name__ == "__main__":
    main()
