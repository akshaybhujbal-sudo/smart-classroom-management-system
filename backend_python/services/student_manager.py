from datetime import datetime

class StudentManager:
    FILENAME = "students.json"

    def __init__(self, storage):
        self.storage = storage

    def _get_all_raw(self):
        loaded = self.storage.load_json(self.FILENAME, default_data={"students": []})
        if isinstance(loaded, dict):
            return loaded.get("students", [])
        return loaded if isinstance(loaded, list) else []

    def _save_all(self, students):
        return self.storage.save_json(self.FILENAME, {"students": students})

    def get_all_students(self, include_inactive=True):
        students = self._get_all_raw()
        if not include_inactive:
            students = [s for s in students if s.get("is_active", True)]
        return {"success": True, "status": 200, "data": students}

    def get_student_by_id(self, student_id):
        students = self._get_all_raw()
        for s in students:
            if s.get("id") == student_id:
                return {"success": True, "status": 200, "data": s}
        return {"success": False, "status": 404, "error": f"Student '{student_id}' not found."}

    def search_students(self, query):
        q = query.strip().lower()
        students = self._get_all_raw()
        matched = []
        for s in students:
            if not s.get("is_active", True):
                continue
            name = s.get("full_name", "").lower()
            roll = s.get("roll_number", "").lower()
            dept = s.get("department", "").lower()
            if q in name or q in roll or q in dept:
                matched.append(s)
        return {"success": True, "status": 200, "data": matched}

    def add_student(self, data):
        roll = data.get("roll_number", "").strip()
        name = data.get("full_name", "").strip()

        if not roll or not name:
            return {"success": False, "status": 400, "error": "Missing required fields: roll_number and full_name are required."}

        students = self._get_all_raw()
        for s in students:
            if s.get("roll_number", "").strip().lower() == roll.lower():
                return {"success": False, "status": 409, "error": f"A student with roll number '{roll}' already exists."}

        max_num = 0
        for s in students:
            sid = s.get("id", "")
            if sid.startswith("std-"):
                try:
                    num = int(sid[4:])
                    if num > max_num:
                        max_num = num
                except ValueError:
                    pass
        new_id = f"std-{max_num + 1:03d}"

        new_student = {
            "id": new_id,
            "roll_number": roll,
            "full_name": name,
            "division": data.get("division", "A"),
            "department": data.get("department", "Computer Science & Engineering"),
            "email": data.get("email", ""),
            "semester": int(data.get("semester", 1)),
            "is_active": True,
            "created_at": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
        }

        students.append(new_student)
        self._save_all(students)
        return {"success": True, "status": 201, "data": new_student, "message": "Student registered successfully."}

    def update_student(self, student_id, data):
        students = self._get_all_raw()
        target = None
        for s in students:
            if s.get("id") == student_id:
                target = s
                break

        if not target:
            return {"success": False, "status": 404, "error": f"Student '{student_id}' not found."}

        new_roll = data.get("roll_number", "").strip()
        if new_roll and new_roll.lower() != target.get("roll_number", "").lower():
            for s in students:
                if s.get("id") != student_id and s.get("roll_number", "").strip().lower() == new_roll.lower():
                    return {"success": False, "status": 409, "error": f"Roll number '{new_roll}' already assigned to another student."}
            target["roll_number"] = new_roll

        if "full_name" in data and data["full_name"].strip():
            target["full_name"] = data["full_name"].strip()
        if "division" in data:
            target["division"] = data["division"]
        if "department" in data:
            target["department"] = data["department"]
        if "email" in data:
            target["email"] = data["email"]
        if "semester" in data:
            target["semester"] = int(data["semester"])
        if "is_active" in data:
            target["is_active"] = bool(data["is_active"])

        self._save_all(students)
        return {"success": True, "status": 200, "data": target, "message": "Student updated successfully."}

    def delete_student(self, student_id):
        students = self._get_all_raw()
        target = None
        for s in students:
            if s.get("id") == student_id:
                target = s
                break

        if not target:
            return {"success": False, "status": 404, "error": f"Student '{student_id}' not found."}

        target["is_active"] = False
        self._save_all(students)
        return {
            "success": True,
            "status": 200,
            "data": {"id": student_id, "status": "deactivated"},
            "message": "Student record deactivated (historical data preserved)."
        }
