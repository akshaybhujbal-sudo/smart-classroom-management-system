# Smart Classroom Management System — Quick Reference Sheet

> **One-page desk reference for demonstration day**

---

## 🚀 Start the Application

```powershell
# Step 1: Navigate to project folder
cd "c:\Users\aaa\Desktop\Smart classroom mangement system"

# Step 2: Launch the server
.\build\smart_classroom_server.exe --port 8080

# Step 3: Open browser
# Navigate to: http://127.0.0.1:8080/
```

## 🧪 Run Tests

```powershell
.\build\smart_classroom_tests.exe
# Expected: 18 PASSED, 0 FAILED
```

## 🔧 Rebuild (if needed)

```powershell
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
cmake -G "MinGW Makefiles" -B build
cmake --build build -j 4
```

---

## 📡 REST API Quick Reference

| Method | Endpoint | Purpose |
|--------|----------|---------|
| GET | `/api/dashboard` | Dashboard summary & KPIs |
| GET | `/api/students` | List all students |
| POST | `/api/students` | Register new student |
| PUT | `/api/students/:id` | Update student |
| DELETE | `/api/students/:id` | Soft deactivate student |
| GET | `/api/attendance?date=YYYY-MM-DD` | Attendance for a date |
| POST | `/api/attendance` | Mark attendance |
| GET | `/api/timetable` | All timetable entries |
| POST | `/api/timetable` | Schedule new lecture |
| DELETE | `/api/timetable/:id` | Remove lecture slot |
| GET | `/api/classroom` | IoT device states |
| PUT | `/api/classroom/devices` | Toggle device |
| GET | `/api/reports/defaulters` | Students below 75% |
| GET | `/api/reports/export-csv` | Download CSV |

---

## 🏗️ System Architecture (One-liner)

```
Browser → HTTP (port 8080) → C++ ApiController → Services → FileStorageManager → JSON files
```

**Layers:**
1. **Presentation** — web/index.html + CSS + JS (SPA)
2. **Controller** — ApiController.cpp (routing, CORS, JSON)
3. **Service** — StudentManager, AttendanceManager, TimetableManager, ClassroomManager, ReportService
4. **Storage** — FileStorageManager (thread-safe, atomic writes)
5. **Data** — data/students.json, attendance.json, timetable.json, devices.json

---

## 🧠 Key Algorithms

| Algorithm | Formula / Method |
|-----------|-----------------|
| Attendance % | `(attended / total) × 100` — returns 0.0 if total = 0 |
| Collision detection | `max(start1, start2) < min(end1, end2)` → overlap |
| Soft deletion | Set `is_active = false` — history preserved |
| Idempotent attendance | Same `student_id + date` → UPDATE (no duplicate) |
| Atomic write | Write to `.tmp` → `std::filesystem::rename()` |
| Thread safety | `std::shared_mutex` — multiple readers, exclusive writer |
| Defaulter threshold | `attendance_percentage < 75.0` |

---

## 📦 Data Models

| Model | Key Fields |
|-------|-----------|
| Student | id, roll_number, full_name, division, semester, email, is_active |
| AttendanceRecord | id, date, student_id, student_roll, status (PRESENT/ABSENT), remarks |
| TimetableEntry | id, subject, teacher, classroom, weekday, start_time, end_time |
| ClassroomDevice | id, device_type (LIGHT/FAN/PROJECTOR), is_powered, level (0-3) |

---

## 📁 Project Structure

```
Smart classroom mangement system/
├── CMakeLists.txt          ← Build config (CMake 3.16+)
├── src/
│   ├── main.cpp            ← Server entry point
│   ├── storage/            ← FileStorageManager
│   ├── services/           ← Business logic (5 services)
│   └── controllers/        ← ApiController (REST routes)
├── include/                ← Header files (models, services, utils)
├── web/                    ← Frontend SPA (HTML/CSS/JS)
├── data/                   ← JSON persistence files
├── tests/                  ← test_backend.cpp (18 tests)
├── docs/                   ← Architecture & requirements docs
└── submission/             ← Report, presentation, viva guide
```

---

## ⚠️ Important Notes

- **IoT is SIMULATED** — No physical hardware connected. Data is mock values in software.
- **No external runtime** — Self-contained C++ executable, no Python/Node.js needed to run
- **No npm** — Frontend uses zero JavaScript frameworks; served directly by C++ server
- **Persistent data** — All changes saved to `data/*.json` files immediately

---

## 🎯 C++ Concepts for Viva

| Concept | Where Used |
|---------|-----------|
| Encapsulation | Each service class (StudentManager, etc.) |
| Composition | ReportService uses 4 other services |
| RAII | `std::shared_ptr<>` for all service instances |
| Templates | `nlohmann::json::get<Student>()` deserialization |
| std::shared_mutex | FileStorageManager reader-writer lock |
| Lambdas | HTTP route handlers in ApiController |
| std::filesystem | Atomic write, directory management |
| Namespaces | `namespace smartclass` throughout |

---

*Print this sheet and keep it at the demo station. See DEMO_GUIDE.md for full walkthrough.*
