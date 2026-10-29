import os
import json
import threading

class FileStorageManager:
    """
    Thread-safe, atomic JSON persistence manager.
    Maintains compatibility with data/*.json files created by the C++ engine.
    """
    def __init__(self, data_dir="data"):
        self.data_dir = data_dir
        self.lock = threading.Lock()
        os.makedirs(self.data_dir, exist_ok=True)

    def _file_path(self, filename: str) -> str:
        return os.path.join(self.data_dir, filename)

    def load_json(self, filename: str, default_data=None):
        if default_data is None:
            default_data = []
        path = self._file_path(filename)
        with self.lock:
            if not os.path.exists(path):
                return default_data
            try:
                with open(path, "r", encoding="utf-8") as f:
                    return json.load(f)
            except Exception as e:
                print(f"[FileStorageManager] Warning: failed to parse {path}: {e}")
                return default_data

    def save_json(self, filename: str, data) -> bool:
        path = self._file_path(filename)
        tmp_path = path + ".tmp"
        with self.lock:
            try:
                with open(tmp_path, "w", encoding="utf-8") as f:
                    json.dump(data, f, indent=2, ensure_ascii=False)
                # Atomic replace
                os.replace(tmp_path, path)
                return True
            except Exception as e:
                print(f"[FileStorageManager] Error writing {path}: {e}")
                if os.path.exists(tmp_path):
                    try:
                        os.remove(tmp_path)
                    except OSError:
                        pass
                return False
