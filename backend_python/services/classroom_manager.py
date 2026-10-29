from datetime import datetime

class ClassroomManager:
    FILENAME = "devices.json"

    def __init__(self, storage):
        self.storage = storage

    def _get_raw(self):
        default_state = {
            "classroom_id": "CR-302",
            "classroom_name": "Smart Class & IoT Lab 302",
            "temperature_celsius": 23.8,
            "occupancy_count": 28,
            "is_simulated": True,
            "last_telemetry_sync": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ"),
            "devices": [
                {"id": "dev-001", "device_type": "LIGHT", "name": "Front Classroom Lights", "is_powered": True, "level": 2, "location": "Front Board Area"},
                {"id": "dev-002", "device_type": "LIGHT", "name": "Rear Seating Lights", "is_powered": True, "level": 2, "location": "Back Wall Area"},
                {"id": "dev-003", "device_type": "FAN", "name": "Main Ceiling Fan Unit", "is_powered": True, "level": 1, "location": "Ceiling Center"},
                {"id": "dev-004", "device_type": "PROJECTOR", "name": "4K Laser Interactive Projector", "is_powered": True, "level": 1, "location": "Ceiling Mount Center"}
            ]
        }
        return self.storage.load_json(self.FILENAME, default_data=default_state)

    def _save_raw(self, state):
        return self.storage.save_json(self.FILENAME, state)

    def get_classroom_status(self):
        state = self._get_raw()
        devices = state.get("devices", [])
        lights_on = sum(1 for d in devices if d.get("device_type") == "LIGHT" and d.get("is_powered"))
        fans_on = sum(1 for d in devices if d.get("device_type") == "FAN" and d.get("is_powered"))
        projector_dev = next((d for d in devices if d.get("device_type") == "PROJECTOR"), None)
        projector_on = bool(projector_dev and projector_dev.get("is_powered"))
        active_count = sum(1 for d in devices if d.get("is_powered"))

        result = dict(state)
        result["lights_on"] = lights_on
        result["fans_on"] = fans_on
        result["projector_on"] = projector_on
        result["active_devices"] = active_count
        result["total_devices"] = len(devices)

        return {"success": True, "status": 200, "data": result}

    def update_device_state(self, device_id, is_powered, level=None):
        if not device_id:
            return {"success": False, "status": 400, "error": "device_id is required."}

        state = self._get_raw()
        devices = state.get("devices", [])
        target = None
        for d in devices:
            if d.get("id") == device_id:
                target = d
                break

        if not target:
            return {"success": False, "status": 404, "error": f"Device '{device_id}' not found."}

        target["is_powered"] = bool(is_powered)
        if level is not None:
            target["level"] = max(0, min(3, int(level)))

        state["last_telemetry_sync"] = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
        self._save_raw(state)

        return {"success": True, "status": 200, "data": target, "message": "Device state updated successfully."}
