class TimetableManager:
    FILENAME = "timetable.json"

    VALID_DAYS = {"MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"}

    def __init__(self, storage):
        self.storage = storage

    def _get_all_raw(self):
        loaded = self.storage.load_json(self.FILENAME, default_data={"schedule": []})
        if isinstance(loaded, dict):
            return loaded.get("schedule", [])
        return loaded if isinstance(loaded, list) else []

    def _save_all(self, entries):
        return self.storage.save_json(self.FILENAME, {"schedule": entries})

    @staticmethod
    def _times_overlap(s1, e1, s2, e2):
        return max(s1, s2) < min(e1, e2)

    def get_all_entries(self):
        entries = self._get_all_raw()
        return {"success": True, "status": 200, "data": entries}

    def get_entries_by_weekday(self, weekday):
        day_up = weekday.strip().upper()
        entries = self._get_all_raw()
        matched = [e for e in entries if e.get("weekday", "").upper() == day_up]
        return {"success": True, "status": 200, "data": matched}

    def create_entry(self, data):
        subject = data.get("subject", "").strip() or data.get("subject_code", "").strip()
        teacher = data.get("teacher", "").strip() or data.get("faculty_name", "").strip()
        classroom = data.get("classroom", "").strip() or data.get("classroom_id", "").strip()
        weekday = (data.get("weekday", "").strip() or data.get("day_of_week", "").strip()).upper()
        start_time = data.get("start_time", "").strip()
        end_time = data.get("end_time", "").strip()

        if not subject or not teacher or not classroom or not weekday or not start_time or not end_time:
            return {"success": False, "status": 400, "error": "Missing required timetable fields."}

        if weekday not in self.VALID_DAYS:
            return {"success": False, "status": 400, "error": f"Invalid weekday '{weekday}'. Must be MONDAY through SATURDAY."}

        if start_time >= end_time:
            return {"success": False, "status": 400, "error": f"start_time ({start_time}) must be strictly before end_time ({end_time})."}

        entries = self._get_all_raw()

        for e in entries:
            if e.get("weekday", "").upper() == weekday:
                e_room = e.get("classroom", "")
                e_teach = e.get("teacher", "")
                e_start = e.get("start_time", "")
                e_end = e.get("end_time", "")

                overlap = self._times_overlap(start_time, end_time, e_start, e_end)
                if overlap:
                    if e_room.lower() == classroom.lower():
                        return {
                            "success": False,
                            "status": 409,
                            "error": f"Collision detected: Classroom '{classroom}' is already booked on {weekday} from {e_start} to {e_end} for '{e.get('subject')}'."
                        }
                    if e_teach.lower() == teacher.lower():
                        return {
                            "success": False,
                            "status": 409,
                            "error": f"Collision detected: Teacher '{teacher}' is already assigned on {weekday} from {e_start} to {e_end} for '{e.get('subject')}'."
                        }

        max_num = 0
        for e in entries:
            eid = e.get("id", "")
            if eid.startswith("tt-"):
                try:
                    num = int(eid[3:])
                    if num > max_num:
                        max_num = num
                except ValueError:
                    pass
        new_id = f"tt-{max_num + 1:03d}"

        new_entry = {
            "id": new_id,
            "subject": subject,
            "teacher": teacher,
            "classroom": classroom,
            "weekday": weekday,
            "start_time": start_time,
            "end_time": end_time
        }

        entries.append(new_entry)
        self._save_all(entries)
        return {"success": True, "status": 201, "data": new_entry, "message": "Timetable slot scheduled successfully."}

    def update_entry(self, entry_id, data):
        entries = self._get_all_raw()
        target = None
        for e in entries:
            if e.get("id") == entry_id:
                target = e
                break

        if not target:
            return {"success": False, "status": 404, "error": f"Timetable entry '{entry_id}' not found."}

        subject = data.get("subject", target.get("subject"))
        teacher = data.get("teacher", target.get("teacher"))
        classroom = data.get("classroom", target.get("classroom"))
        weekday = data.get("weekday", target.get("weekday")).upper()
        start_time = data.get("start_time", target.get("start_time"))
        end_time = data.get("end_time", target.get("end_time"))

        if start_time >= end_time:
            return {"success": False, "status": 400, "error": "start_time must be strictly before end_time."}

        for e in entries:
            if e.get("id") != entry_id and e.get("weekday", "").upper() == weekday:
                if self._times_overlap(start_time, end_time, e.get("start_time", ""), e.get("end_time", "")):
                    if e.get("classroom", "").lower() == classroom.lower():
                        return {"success": False, "status": 409, "error": f"Collision: classroom '{classroom}' busy."}
                    if e.get("teacher", "").lower() == teacher.lower():
                        return {"success": False, "status": 409, "error": f"Collision: teacher '{teacher}' busy."}

        target["subject"] = subject
        target["teacher"] = teacher
        target["classroom"] = classroom
        target["weekday"] = weekday
        target["start_time"] = start_time
        target["end_time"] = end_time

        self._save_all(entries)
        return {"success": True, "status": 200, "data": target, "message": "Timetable slot updated successfully."}

    def delete_entry(self, entry_id):
        entries = self._get_all_raw()
        idx = next((i for i, e in enumerate(entries) if e.get("id") == entry_id), -1)
        if idx == -1:
            return {"success": False, "status": 404, "error": f"Timetable entry '{entry_id}' not found."}

        entries.pop(idx)
        self._save_all(entries)
        return {"success": True, "status": 200, "data": {"id": entry_id, "status": "deleted"}, "message": "Timetable slot removed."}
