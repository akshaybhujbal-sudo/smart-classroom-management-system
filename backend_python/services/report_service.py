from datetime import datetime

class ReportService:
    def __init__(self, storage, student_manager, attendance_manager, timetable_manager, classroom_manager):
        self.storage = storage
        self.student_mgr = student_manager
        self.attendance_mgr = attendance_manager
        self.timetable_mgr = timetable_manager
        self.classroom_mgr = classroom_manager

    def get_dashboard_summary(self):
        now = datetime.utcnow()
        today_str = now.strftime("%Y-%m-%d")

        # Students stats
        all_students_res = self.student_mgr.get_all_students(include_inactive=True)
        all_students = all_students_res.get("data", [])
        active_students = [s for s in all_students if s.get("is_active", True)]
        inactive_students = [s for s in all_students if not s.get("is_active", True)]

        # Today attendance
        today_att_res = self.attendance_mgr.get_attendance_by_date(today_str)
        today_records = today_att_res.get("data", [])
        today_present = sum(1 for r in today_records if r.get("status") == "PRESENT")
        today_absent = sum(1 for r in today_records if r.get("status") == "ABSENT")
        today_logged = len(today_records)
        today_pct = 0.0 if today_logged == 0 else round((today_present / today_logged) * 100.0, 1)

        # Classroom stats
        cls_res = self.classroom_mgr.get_classroom_status()
        classroom_data = cls_res.get("data", {})

        # Defaulters count & per-student statistics
        all_att_records = self.attendance_mgr._get_all_raw()
        student_att_counts = {}
        for r in all_att_records:
            sid = r.get("student_id")
            if sid not in student_att_counts:
                student_att_counts[sid] = {"total": 0, "attended": 0}
            student_att_counts[sid]["total"] += 1
            if r.get("status") == "PRESENT":
                student_att_counts[sid]["attended"] += 1

        defaulters_count = 0
        for s in active_students:
            sid = s.get("id")
            counts = student_att_counts.get(sid, {"total": 0, "attended": 0})
            if counts["total"] > 0:
                pct = (counts["attended"] / counts["total"]) * 100.0
                if pct < 75.0:
                    defaulters_count += 1

        # Attendance trend grouped by date (sorted)
        date_groups = {}
        for r in all_att_records:
            d = r.get("date")
            if not d:
                continue
            if d not in date_groups:
                date_groups[d] = {"present": 0, "absent": 0, "total": 0}
            date_groups[d]["total"] += 1
            if r.get("status") == "PRESENT":
                date_groups[d]["present"] += 1
            elif r.get("status") == "ABSENT":
                date_groups[d]["absent"] += 1

        sorted_dates = sorted(date_groups.keys())
        attendance_trend = []
        for d in sorted_dates:
            g = date_groups[d]
            pct = 0.0 if g["total"] == 0 else round((g["present"] / g["total"]) * 100.0, 1)
            attendance_trend.append({
                "date": d,
                "present": g["present"],
                "absent": g["absent"],
                "total": g["total"],
                "percentage": pct
            })

        latest_attendance = attendance_trend[-1] if attendance_trend else {
            "date": today_str,
            "present": 0,
            "absent": 0,
            "percentage": 0.0,
            "total_logged": 0
        }

        return {
            "success": True,
            "status": 200,
            "data": {
                "current_date": today_str,
                "timestamp": now.strftime("%Y-%m-%dT%H:%M:%SZ"),
                "system_status": "ONLINE",
                "students": {
                    "total": len(all_students),
                    "active": len(active_students),
                    "inactive": len(inactive_students),
                    "defaulters_count": defaulters_count
                },
                "today_attendance": {
                    "present": today_present,
                    "absent": today_absent,
                    "total_logged": today_logged,
                    "percentage": today_pct
                },
                "latest_attendance": latest_attendance,
                "attendance_trend": attendance_trend,
                "classroom": classroom_data
            }
        }

    def get_student_statistics_report(self):
        all_students_res = self.student_mgr.get_all_students(include_inactive=True)
        students = all_students_res.get("data", [])
        all_records = self.attendance_mgr._get_all_raw()

        counts = {}
        for r in all_records:
            sid = r.get("student_id")
            if sid not in counts:
                counts[sid] = {"total": 0, "attended": 0}
            counts[sid]["total"] += 1
            if r.get("status") == "PRESENT":
                counts[sid]["attended"] += 1

        report = []
        for s in students:
            sid = s.get("id")
            c = counts.get(sid, {"total": 0, "attended": 0})
            total = c["total"]
            attended = c["attended"]
            pct = 0.0 if total == 0 else round((attended / total) * 100.0, 1)
            report.append({
                "student_id": sid,
                "roll_number": s.get("roll_number", ""),
                "full_name": s.get("full_name", ""),
                "division": s.get("division", "A"),
                "total_classes": total,
                "attended": attended,
                "absent": total - attended,
                "percentage": pct,
                "is_defaulter": pct < 75.0 and total > 0
            })

        return {"success": True, "status": 200, "data": {"students": report, "total": len(report)}}

    def get_defaulters_report(self, threshold=75.0):
        stats_res = self.get_student_statistics_report()
        all_stats = stats_res["data"]["students"]
        defaulters = [s for s in all_stats if s["percentage"] < threshold and s["total_classes"] > 0]
        return {
            "success": True,
            "status": 200,
            "data": {
                "threshold": threshold,
                "count": len(defaulters),
                "defaulters": defaulters
            }
        }

    def export_students_csv(self):
        students_res = self.student_mgr.get_all_students(include_inactive=True)
        students = students_res.get("data", [])
        lines = ["id,roll_number,full_name,division,department,semester,is_active,created_at"]
        for s in students:
            lines.append(f"{s.get('id')},{s.get('roll_number')},\"{s.get('full_name')}\",{s.get('division')},\"{s.get('department')}\",{s.get('semester')},{s.get('is_active')},{s.get('created_at')}")
        return "\n".join(lines)

    def export_attendance_csv(self, date_filter=""):
        records = self.attendance_mgr._get_all_raw()
        if date_filter:
            records = [r for r in records if r.get("date") == date_filter]
        lines = ["id,date,student_id,student_roll,status,recorded_at,remarks"]
        for r in records:
            lines.append(f"{r.get('id')},{r.get('date')},{r.get('student_id')},{r.get('student_roll')},{r.get('status')},{r.get('recorded_at')},\"{r.get('remarks', '')}\"")
        return "\n".join(lines)
