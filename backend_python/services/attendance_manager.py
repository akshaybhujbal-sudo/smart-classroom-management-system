from datetime import datetime

class AttendanceManager:
    FILENAME = "attendance.json"

    def __init__(self, storage, student_manager):
        self.storage = storage
        self.student_manager = student_manager

    def _get_all_raw(self):
        loaded = self.storage.load_json(self.FILENAME, default_data={"records": []})
        if isinstance(loaded, dict):
            return loaded.get("records", [])
        return loaded if isinstance(loaded, list) else []

    def _save_all(self, records):
        return self.storage.save_json(self.FILENAME, {"records": records})

    def get_attendance_by_date(self, date_str):
        records = self._get_all_raw()
        matched = [r for r in records if r.get("date") == date_str]
        return {"success": True, "status": 200, "data": matched}

    def get_student_attendance_history(self, student_id):
        records = self._get_all_raw()
        matched = [r for r in records if r.get("student_id") == student_id]
        total = len(matched)
        attended = sum(1 for r in matched if r.get("status") == "PRESENT")
        percentage = 0.0 if total == 0 else round((attended / total) * 100.0, 1)

        return {
            "success": True,
            "status": 200,
            "data": {
                "student_id": student_id,
                "records": matched,
                "total_classes": total,
                "attended": attended,
                "percentage": percentage
            }
        }

    def get_daily_attendance_summary(self, date_str):
        records = self._get_all_raw()
        matched = [r for r in records if r.get("date") == date_str]
        total = len(matched)
        present = sum(1 for r in matched if r.get("status") == "PRESENT")
        absent = total - present
        percentage = 0.0 if total == 0 else round((present / total) * 100.0, 1)

        return {
            "success": True,
            "status": 200,
            "data": {
                "date": date_str,
                "total": total,
                "present": present,
                "absent": absent,
                "percentage": percentage,
                "records": matched
            }
        }

    def record_attendance(self, data):
        student_id = data.get("student_id", "").strip()
        date_str = data.get("date", "").strip()
        status = data.get("status", "").strip().upper()

        if not student_id or not date_str or not status:
            return {"success": False, "status": 400, "error": "Missing required fields: student_id, date, status."}

        if status not in ("PRESENT", "ABSENT"):
            return {"success": False, "status": 400, "error": "Invalid attendance status. Must be 'PRESENT' or 'ABSENT'."}

        try:
            datetime.strptime(date_str, "%Y-%m-%d")
        except ValueError:
            return {"success": False, "status": 400, "error": "Invalid date format. Expected YYYY-MM-DD."}

        records = self._get_all_raw()

        for r in records:
            if r.get("student_id") == student_id and r.get("date") == date_str:
                r["status"] = status
                if "remarks" in data:
                    r["remarks"] = data["remarks"]
                r["recorded_at"] = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
                self._save_all(records)
                return {"success": True, "status": 200, "data": r, "message": "Attendance record updated."}

        max_num = 0
        for r in records:
            rid = r.get("id", "")
            if rid.startswith("att-"):
                try:
                    num = int(rid[4:])
                    if num > max_num:
                        max_num = num
                except ValueError:
                    pass
        new_id = f"att-{max_num + 1:03d}"

        roll = data.get("student_roll", "")
        if not roll:
            s_res = self.student_manager.get_student_by_id(student_id)
            if s_res.get("success"):
                roll = s_res["data"].get("roll_number", "")

        new_record = {
            "id": new_id,
            "date": date_str,
            "student_id": student_id,
            "student_roll": roll,
            "status": status,
            "recorded_at": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ"),
            "remarks": data.get("remarks", "")
        }

        records.append(new_record)
        self._save_all(records)
        return {"success": True, "status": 200, "data": new_record, "message": "Attendance marked successfully."}

    def update_attendance(self, record_id, status, remarks=None):
        records = self._get_all_raw()
        target = None
        for r in records:
            if r.get("id") == record_id:
                target = r
                break

        if not target:
            return {"success": False, "status": 404, "error": f"Attendance record '{record_id}' not found."}

        if status:
            s_up = status.strip().upper()
            if s_up not in ("PRESENT", "ABSENT"):
                return {"success": False, "status": 400, "error": "Invalid attendance status."}
            target["status"] = s_up

        if remarks is not None:
            target["remarks"] = remarks

        self._save_all(records)
        return {"success": True, "status": 200, "data": target, "message": "Attendance record updated."}
