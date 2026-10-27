# Smart Classroom Management System
## College Engineering Mini-Project Report

| Field | Details |
|---|---|
| **Title** | Smart Classroom Management System |
| **Subject** | [PLACEHOLDER: Subject Name, e.g., "Mini-Project / Application Development"] |
| **Department** | [PLACEHOLDER: Department Name] |
| **Institution** | [PLACEHOLDER: College Name] |
| **Submitted by** | [PLACEHOLDER: Student Name] \| Roll No: [PLACEHOLDER: Roll No] \| Div: [PLACEHOLDER: Division] |
| **Guide** | [PLACEHOLDER: Guide Name, Designation] |
| **Academic Year** | [PLACEHOLDER: e.g., 2025–2026] |
| **Semester** | [PLACEHOLDER: e.g., 6th] |

---

## Table of Contents

1. [Abstract](#a-abstract)
2. [Introduction](#b-introduction)
3. [Literature Review / Related Work](#c-literature-review--related-work)
4. [System Requirements](#d-system-requirements)
5. [System Design and Architecture](#e-system-design-and-architecture)
6. [Data Model](#f-data-model)
7. [REST API Design](#g-rest-api-design)
8. [Implementation Details](#h-implementation-details)
9. [Testing](#i-testing)
10. [Results and Screenshots](#j-results-and-screenshots)
11. [Limitations and Future Work](#k-limitations-and-future-work)
12. [Conclusion](#l-conclusion)
13. [References](#m-references)

---

## A. Abstract

The Smart Classroom Management System is a self-contained web application built in C++17 that digitises the day-to-day administrative tasks of an engineering classroom. The system provides a multi-view Single Page Application (SPA) served directly from a C++ HTTP server on port 8080, offering features including student registration and lifecycle management, daily attendance tracking with automated defaulter detection, lecture timetable scheduling with room and teacher collision prevention, simulated IoT classroom device control, and report generation with CSV export.

The backend is implemented entirely in C++17, using the cpp-httplib header-only library for HTTP serving and nlohmann/json for structured data persistence via JSON flat-files. Thread safety is maintained through `std::shared_mutex` with atomic file writes. The frontend uses HTML5, CSS3, and Vanilla JavaScript with zero npm dependencies. All 18 planned unit tests and the full integration test suite passed on 2026-10-22, demonstrating a production-ready mini-project that effectively demonstrates object-oriented programming, RESTful API design, and systems-level C++ engineering.

---

## B. Introduction

### Problem Statement

Traditional classroom management in Indian engineering colleges relies heavily on manual processes: paper attendance registers that must be tallied at the end of each month, handwritten timetable charts posted on notice boards, and manual generation of defaulter lists before examinations. These workflows are error-prone, time-consuming, and make real-time data access impossible. Attendance sheets can be lost or tampered with. Computing a student's attendance percentage across multiple subjects is an entirely manual arithmetic exercise. There is no structured mechanism for tracking which classroom devices (fans, lights, projectors) are active, nor is there a consolidated dashboard for an administrator to see the state of the classroom at a glance.

### Motivation for C++ Backend

Most mini-project submissions at this level default to Python/Flask or Node.js/Express for backend development. This project deliberately chose C++17 as the implementation language for the following reasons:

- **Systems Programming Education:** C++ exposes developers to memory management, pointer semantics, and compile-time constructs not visible in managed-runtime languages.
- **Performance:** C++ executables start in milliseconds and have substantially lower memory overhead than JVM- or Python-runtime-based servers.
- **OOP Depth:** C++ OOP — including RAII, templates, operator overloading, and manual lifetime management — demonstrates concepts at a level of depth that satisfies advanced coursework objectives.
- **Portability:** The CMake build system combined with LLVM-MinGW produces a single self-contained `.exe` requiring no runtime installation on the host machine.

### Scope of the Project

**In Scope:**
- Student registration, update, search, and soft deletion
- Daily attendance recording, retrieval, and percentage computation
- Lecture timetable management with conflict detection
- Simulated IoT device control for a single classroom (CR-302)
- Dashboard with attendance trend analytics
- Report generation and CSV export
- A browser-based SPA served directly from the C++ binary

**Out of Scope:**
- Physical IoT hardware integration (Arduino, ESP32, Raspberry Pi) — **all device control is software simulation**
- Multi-classroom management
- User authentication and role-based access control
- Cloud deployment
- Mobile application

### Objectives

1. Implement a fully functional REST API backend in C++17 with clean layered architecture.
2. Enable digital student registration with validation, duplicate prevention, and soft deletion.
3. Provide idempotent daily attendance recording with automated percentage calculation and defaulter detection (threshold: < 75%).
4. Implement lecture timetable scheduling with room-level and teacher-level collision detection.
5. Simulate IoT classroom device management (lights, fans, projector) with state persistence.
6. Generate student and attendance reports, including a downloadable CSV export.
7. Serve a fully functional SPA frontend with zero external npm dependencies from the C++ server itself.
8. Achieve 100% pass rate on a comprehensive 18-scenario test suite covering edge cases such as duplicate entries, division-by-zero, invalid JSON payloads, and post-restart persistence.

---

## C. Literature Review / Related Work

### Similar Systems

Several existing platforms address classroom management:

- **Google Classroom** and **Microsoft Teams for Education** provide course management and attendance features but require cloud connectivity, institutional accounts, and do not expose a programmable REST API for custom integration.
- **Fedena** and **iGradePlus** are open-source school ERP systems built on Ruby on Rails or PHP. They are comprehensive but heavyweight, requiring database servers and complex deployment.
- **Arduino/Raspberry Pi IoT classroom projects** appearing in IEEE conference proceedings typically focus exclusively on sensor data acquisition (temperature, occupancy via PIR sensors) with no student or timetable management layer.

This project differs by combining all three concerns — academic management, timetable administration, and IoT device control — into a single, self-contained C++ executable with no external runtime dependencies.

### Why REST API Over Alternatives?

REST (Representational State Transfer) was chosen over alternatives such as GraphQL or WebSockets for the following reasons:

- REST maps naturally to HTTP verbs (GET, POST, PUT, DELETE), making the API immediately understandable and testable with standard tools like `curl` or a browser.
- The stateless nature of REST simplifies server implementation — no session state needs to be maintained between requests.
- REST is the dominant API style in industry and aligns with the educational objectives of the course.
- GraphQL would add implementation complexity (schema definition language, query parsing) disproportionate to the project's scope.

### Why JSON Flat-Files Over an RDBMS?

For a mini-project with modest data volumes (hundreds of students, thousands of attendance records), a full relational database such as MySQL or PostgreSQL introduces substantial operational overhead:

- A separate database server process must be running and configured.
- Connection pooling, schema migrations, and SQL query construction add code complexity.
- Portability suffers — the project would no longer be a single self-contained executable.

JSON flat-files with atomic writes and `std::shared_mutex` reader-writer locking provide sufficient concurrency safety for a single-server academic management system while keeping the build and deployment process trivially simple. The trade-off (no ad-hoc querying, linear-scan search) is acceptable given the dataset sizes involved.

---

## D. System Requirements

### Hardware Requirements (Minimum)

| Component | Minimum Specification |
|---|---|
| Processor | Intel Core i3 or equivalent (64-bit) |
| RAM | 2 GB (4 GB recommended) |
| Storage | 200 MB free disk space (build artifacts + data) |
| Network | Localhost loopback only (no internet required at runtime) |
| Display | 1280 × 720 resolution (for frontend UI) |

### Software Requirements (Build Environment)

| Software | Version Used | Purpose |
|---|---|---|
| CMake | 4.4.4 | Build system generator |
| LLVM-MinGW Clang (UCRT) | 23.1.3 | C++17 compiler toolchain |
| C++ Standard | ISO/IEC 14882:2017 (C++17) | Language standard |
| cpp-httplib | Latest (header-only) | HTTP server library |
| nlohmann/json | Latest (header-only) | JSON serialisation |
| Modern Web Browser | Chrome 100+ / Firefox 100+ / Edge 100+ | Frontend client |

### Runtime Dependencies

**None.** The compiled executable is fully self-contained. No Python interpreter, no JVM, no Node.js runtime, and no database server are required to run the application. The browser used to access the frontend is the only external software needed at runtime.

---

## E. System Design and Architecture

### Layered Architecture

The system follows a strict five-layer architecture:

```
┌─────────────────────────────────────────────────────────────┐
│                   PRESENTATION LAYER                        │
│         HTML5 SPA  ·  CSS3  ·  Vanilla JavaScript          │
│         (web/index.html  — served from C++ binary)          │
├─────────────────────────────────────────────────────────────┤
│                   CONTROLLER LAYER                          │
│            src/controllers/ApiController.cpp                │
│      HTTP Routing  ·  CORS Headers  ·  JSON Parsing        │
├─────────────────────────────────────────────────────────────┤
│                    SERVICE LAYER                            │
│  StudentManager  │  AttendanceManager  │  TimetableManager  │
│  ClassroomManager  │  ReportService                        │
├─────────────────────────────────────────────────────────────┤
│                    STORAGE LAYER                            │
│              src/storage/FileStorageManager.cpp             │
│   std::shared_mutex  ·  Atomic Writes  ·  JSON Parsing     │
├─────────────────────────────────────────────────────────────┤
│                      DATA LAYER                             │
│  data/students.json  ·  data/attendance.json               │
│  data/timetable.json  ·  data/devices.json                 │
└─────────────────────────────────────────────────────────────┘
```

### Module Descriptions

| Module | Source File | Responsibility |
|---|---|---|
| Entry Point | `src/main.cpp` | Server bootstrap, signal handling (SIGINT/SIGTERM), port binding |
| API Controller | `src/controllers/ApiController.cpp` | URL routing, CORS header injection, request body parsing, JSON response serialisation |
| Student Manager | `src/services/StudentManager.cpp` | Student CRUD operations, roll number uniqueness validation, soft deletion (is_active flag) |
| Attendance Manager | `src/services/AttendanceManager.cpp` | Attendance recording, idempotent upsert, percentage calculation, date/status validation |
| Timetable Manager | `src/services/TimetableManager.cpp` | Lecture slot creation, interval-overlap collision detection for room and teacher |
| Classroom Manager | `src/services/ClassroomManager.cpp` | Simulated IoT device state management, temperature and occupancy simulation |
| Report Service | `src/services/ReportService.cpp` | Dashboard aggregation, defaulter list computation, CSV export |
| File Storage Manager | `src/storage/FileStorageManager.cpp` | Thread-safe JSON read/write, atomic file replacement via `.tmp` rename |

### Data Flow: POST /api/attendance (Typical Request)

```
Browser (fetch POST /api/attendance)
        │
        ▼
ApiController::registerRoutes()
  → Parses JSON body from httplib::Request
  → Validates Content-Type
        │
        ▼
AttendanceManager::recordAttendance()
  → Validates date format (YYYY-MM-DD)
  → Validates status (PRESENT | ABSENT)
  → Checks for existing record (same student_id + date)
     ├─ EXISTS  → UPDATE in-place (idempotent upsert)
     └─ NEW     → Assign ID (att- + counter), append record
        │
        ▼
FileStorageManager::saveCollection()
  → Serialises vector<AttendanceRecord> to JSON
  → Writes to data/attendance.json.tmp
  → std::filesystem::rename() → data/attendance.json
        │
        ▼
ApiController returns HTTP 200 / 201
  → {success: true, status: 200, data: {...}, message: "..."}
        │
        ▼
Browser receives JSON, updates UI without page reload
```

### Design Patterns Used

| Pattern | Where Applied |
|---|---|
| **RAII** | All service instances managed via `std::shared_ptr`; resources released automatically on destruction |
| **Composition** | `ReportService` holds `shared_ptr` to all four other service managers |
| **Reader-Writer Lock** | `std::shared_mutex` in `FileStorageManager`; multiple concurrent readers, exclusive writer |
| **Atomic Write Pattern** | Write to `.tmp` file, then `std::filesystem::rename()` — prevents corrupt partial writes |
| **Soft Delete** | Student deactivation sets `is_active=false`; records are never physically removed |
| **Idempotent Upsert** | Attendance recording checks for existing (student_id, date) pair before inserting |

---

## F. Data Model

### Entity: Student

| Field | Type | Constraints | Description |
|---|---|---|---|
| `id` | string | PK, `std-` prefix | Auto-generated unique identifier |
| `roll_number` | string | Unique, Not Null | Student roll number (e.g., CS2026-901) |
| `full_name` | string | Not Null | Full legal name |
| `division` | string | Not Null | Class division (e.g., A, B) |
| `department` | string | Not Null | Academic department (e.g., Computer Engineering) |
| `email` | string | Not Null | Email address |
| `semester` | integer | 1–8 | Current semester |
| `is_active` | boolean | Default: true | False after soft deletion |
| `created_at` | string | ISO 8601 | Timestamp of registration |

### Entity: AttendanceRecord

| Field | Type | Constraints | Description |
|---|---|---|---|
| `id` | string | PK, `att-` prefix | Auto-generated unique identifier |
| `date` | string | YYYY-MM-DD | Date of attendance |
| `student_id` | string | FK → Student.id | Reference to student |
| `student_roll` | string | Denormalised | Roll number (for display without join) |
| `status` | string | PRESENT \| ABSENT | Attendance status |
| `recorded_at` | string | ISO 8601 | Timestamp when recorded |
| `remarks` | string | Optional | Additional notes |

### Entity: TimetableEntry

| Field | Type | Constraints | Description |
|---|---|---|---|
| `id` | string | PK, `tt-` prefix | Auto-generated unique identifier |
| `subject` | string | Not Null | Subject name |
| `teacher` | string | Not Null | Teacher's name |
| `classroom` | string | Not Null | Room identifier (e.g., CR-302) |
| `weekday` | string | MON–SUN enum | Day of the week |
| `start_time` | string | HH:MM | Lecture start time |
| `end_time` | string | HH:MM | Lecture end time |

### Entity: ClassroomDevice

| Field | Type | Constraints | Description |
|---|---|---|---|
| `id` | string | PK, `dev-` prefix | Auto-generated unique identifier |
| `device_type` | string | LIGHT \| FAN \| PROJECTOR | Category of device |
| `name` | string | Not Null | Friendly name (e.g., "Front Light") |
| `is_powered` | boolean | Default: false | Current power state |
| `level` | integer | 0–3 | Intensity/speed setting |
| `location` | string | Not Null | Physical location label |

### Entity Relationships

```
Student ──< AttendanceRecord    (one student → many attendance records)
TimetableEntry (independent, references classroom string)
ClassroomDevice ──> ClassroomState (devices[] aggregated in state object)
```

### Soft Deletion Strategy

When a student is deactivated via `DELETE /api/students/:id`, the system sets `is_active = false` on the student record. **No attendance records are deleted.** The student's historical attendance data remains fully intact and is included in historical reports. The student no longer appears in active roster lists but is preserved in the JSON file. This prevents data loss while logically removing the student from day-to-day operations.

### Idempotent Attendance Upsert

Before inserting a new attendance record, `AttendanceManager` performs a lookup for an existing record matching both `student_id` and `date`. If found, the existing record is updated in place (status, remarks, recorded_at are refreshed). If not found, a new record is created. This ensures that re-submitting an attendance entry never creates duplicate records in the data file.

### Storage Format

Each entity collection is stored as a JSON array in a dedicated flat-file:

```
data/
├── students.json      → [ { Student }, ... ]
├── attendance.json    → [ { AttendanceRecord }, ... ]
├── timetable.json     → [ { TimetableEntry }, ... ]
└── devices.json       → [ { ClassroomDevice }, ... ]
```

---

## G. REST API Design

### Response Envelope Format

All API responses follow a consistent envelope structure:

```json
{
  "success": true,
  "status": 200,
  "data": { ... },
  "message": "Operation completed successfully"
}
```

Error responses use the same envelope with `success: false` and an appropriate HTTP status code.

### Standard HTTP Error Codes

| Code | Meaning | Example Trigger |
|---|---|---|
| 400 | Bad Request | Missing required field, invalid date format, malformed JSON |
| 404 | Not Found | `GET /api/students/std-999` where id does not exist |
| 409 | Conflict | Duplicate roll number, timetable slot collision |
| 500 | Internal Server Error | File I/O failure during persistence |

### Complete Endpoint Reference

| Method | Path | Description | Request Body | Response Data |
|---|---|---|---|---|
| GET | `/` | Serves SPA (web/index.html) | — | HTML |
| **Dashboard** | | | | |
| GET | `/api/dashboard` | Aggregated summary | — | KPI stats, attendance_trend[], today schedule, classroom state |
| **Students** | | | | |
| GET | `/api/students` | List all active students | — | `[Student]` |
| GET | `/api/students/:id` | Get student by ID | — | `Student` |
| POST | `/api/students` | Create new student | `{roll_number, full_name, division, department, email, semester}` | Created `Student` |
| PUT | `/api/students/:id` | Update student fields | Partial `Student` fields | Updated `Student` |
| DELETE | `/api/students/:id` | Soft-deactivate student | — | `{message}` |
| **Attendance** | | | | |
| GET | `/api/attendance?date=YYYY-MM-DD` | Attendance for a date | — | `[AttendanceRecord]` |
| GET | `/api/attendance?student=rollno` | Attendance by roll number | — | `[AttendanceRecord]` |
| GET | `/api/attendance/student/:id` | Full history for student | — | `[AttendanceRecord]` with percentage |
| POST | `/api/attendance` | Record/update attendance | `{student_id, date, status, remarks?}` | `AttendanceRecord` |
| PUT | `/api/attendance/:id` | Update existing record | Partial `AttendanceRecord` | Updated record |
| **Timetable** | | | | |
| GET | `/api/timetable` | All timetable entries | — | `[TimetableEntry]` |
| GET | `/api/timetable?day=MONDAY` | Entries for one weekday | — | `[TimetableEntry]` |
| POST | `/api/timetable` | Create lecture slot | `{subject, teacher, classroom, weekday, start_time, end_time}` | Created `TimetableEntry` or 409 |
| PUT | `/api/timetable/:id` | Update lecture slot | Partial `TimetableEntry` | Updated entry |
| DELETE | `/api/timetable/:id` | Remove lecture slot | — | `{message}` |
| **Classroom (IoT)** | | | | |
| GET | `/api/classroom` | Current classroom state | — | `ClassroomState` with `devices[]` |
| PUT | `/api/classroom/devices` | Update device state | `{device_id, is_powered, level}` | Updated `ClassroomDevice` |
| POST | `/api/classroom/devices` | Add new device | `{device_type, name, location, level}` | Created `ClassroomDevice` |
| **Reports** | | | | |
| GET | `/api/reports/students` | All students report | — | Students with attendance % |
| GET | `/api/reports/defaulters` | Students below 75% | — | `[Student]` with percentages |
| GET | `/api/reports/attendance` | Full attendance ledger | — | All records |
| GET | `/api/reports/export-csv?type=attendance` | Download attendance CSV | — | `text/csv` file |
| GET | `/api/reports/export-csv?type=students` | Download students CSV | — | `text/csv` file |

---

## H. Implementation Details

### Key OOP Concepts

**Encapsulation:** Each service class (`StudentManager`, `AttendanceManager`, `TimetableManager`, `ClassroomManager`, `ReportService`) encapsulates its domain data and logic entirely. External callers interact only through public member functions; internal data structures and persistence calls are private.

**Abstraction:** `FileStorageManager` provides an abstract persistence interface. Service classes call `loadCollection<T>()` and `saveCollection<T>()` without knowing the physical file path, serialisation format details, or locking strategy.

**Composition:** `ReportService` is constructed by injecting `shared_ptr` instances of all four other service managers. It aggregates their data without inheriting from them, demonstrating composition-over-inheritance.

**RAII:** All service instances are wrapped in `std::shared_ptr`. When the server shuts down and shared pointers go out of scope, destructors are automatically invoked and resources are released without manual cleanup.

**Templates:** The nlohmann/json library's `get<T>()` template function is used throughout for type-safe JSON deserialisation. For example: `j.at("semester").get<int>()` provides compile-time type checking.

### FileStorageManager: Thread Safety and Atomic Writes

The `FileStorageManager` uses a `std::shared_mutex` to implement a reader-writer lock:

- **Read operations** (`loadCollection`) acquire a `std::shared_lock`, allowing multiple threads to read simultaneously.
- **Write operations** (`saveCollection`) acquire a `std::unique_lock`, providing exclusive access and blocking all concurrent readers and writers.

Atomic write procedure:
1. Serialise data to a JSON string in memory.
2. Write JSON string to `<filename>.json.tmp`.
3. Call `std::filesystem::rename("<filename>.json.tmp", "<filename>.json")`.

This ensures that if the process is killed during a write, the original data file is never left in a partial/corrupt state. The rename operation is atomic at the OS level on NTFS.

### Attendance Percentage Algorithm

```
percentage = (present_count / total_records) × 100
```

Rounded to one decimal place. Division-by-zero is explicitly guarded:

```
if (total_records == 0) return 0.0;
```

**Verified result:** A student with 1 PRESENT out of 2 total records correctly yields 50.0%. A student with 0 total records correctly yields 0.0% (no division-by-zero exception).

### Timetable Collision Detection

For any two candidate timetable entries on the same `weekday`, a collision exists if and only if their time intervals overlap. The algorithm uses the standard interval-overlap formula:

```
collision = (max(start_A, start_B) < min(end_A, end_B))
```

Times are compared as HH:MM strings lexicographically (valid because the format is zero-padded and fixed-width).

A collision is flagged if:
- **Same classroom AND same time overlap**, OR
- **Same teacher AND same time overlap**

This prevents both double-booking a room and scheduling a teacher in two places simultaneously.

### ID Generation

IDs are generated using a prefix concatenated with a sequential counter:

| Entity | Prefix | Example |
|---|---|---|
| Student | `std-` | `std-012` |
| Attendance | `att-` | `att-047` |
| Timetable | `tt-` | `tt-302` |
| Device | `dev-` | `dev-001` |

The counter is derived by scanning the existing collection for the highest numeric suffix on startup, ensuring IDs remain unique across server restarts.

### Frontend–Backend Integration

The SPA (`web/index.html`) communicates with the backend exclusively via the browser's native `fetch()` API. All requests target `http://localhost:8080/api/...`. The `ApiController` injects `Access-Control-Allow-Origin: *` and appropriate `Access-Control-Allow-Methods` / `Access-Control-Allow-Headers` CORS headers on every response, enabling the browser to make cross-origin requests during development. The frontend parses the JSON response envelope and updates the DOM dynamically without page reloads.

---

## I. Testing

### Test Strategy

A dedicated test runner was implemented in `test_backend.cpp`. Tests are executed against a live server instance and verify the complete request-response cycle — from HTTP routing through service logic to storage persistence and back. This constitutes functional/integration testing at the API boundary. Tests are ordered to exercise dependencies sequentially (e.g., create a student before recording attendance for that student).

### Unit / Functional Test Results (2026-10-22)

**Overall: 18/18 PASS**

| # | Test Scenario | Method | Endpoint | Expected | Result |
|---|---|---|---|---|---|
| 1 | Add a valid student | POST | /api/students | 201, ID generated | ✅ PASS (ID: std-012) |
| 2 | Reject student with missing required fields | POST | /api/students | 400 Bad Request | ✅ PASS |
| 3 | Reject duplicate roll number | POST | /api/students | 409 Conflict | ✅ PASS |
| 4 | Retrieve and search student records | GET | /api/students | 200, matching record | ✅ PASS (Found CS2026-901) |
| 5 | Update a student successfully | PUT | /api/students/:id | 200, updated fields | ✅ PASS (name & division updated) |
| 6 | Return 404 for nonexistent student | GET | /api/students/:id | 404 Not Found | ✅ PASS |
| 7 | Record attendance successfully | POST | /api/attendance | 201, PRESENT record | ✅ PASS (2026-10-21) |
| 8 | Reject duplicate attendance (idempotent update) | POST | /api/attendance | 200, no duplication | ✅ PASS |
| 9 | Calculate attendance percentages correctly | GET | /api/attendance/student/:id | 50.0% & 0.0% | ✅ PASS |
| 10 | Reject invalid date or attendance status | POST | /api/attendance | 400 Bad Request | ✅ PASS |
| 11 | Create a valid timetable entry | POST | /api/timetable | 201, tt-302 created | ✅ PASS (14:00–15:30) |
| 12 | Reject conflicting classroom booking | POST | /api/timetable | 409 Collision | ✅ PASS |
| 13 | Update and delete timetable entries | PUT/DELETE | /api/timetable/:id | 200 success | ✅ PASS |
| 14 | Update simulated device states | PUT | /api/classroom/devices | 200, state updated | ✅ PASS (power ON, level 2) |
| 15 | Persisted data survives server restart | GET | /api/students | 200, record loaded | ✅ PASS |
| 16 | Deactivating student preserves attendance history | DELETE | /api/students/:id | is_active=false, history intact | ✅ PASS (2 records preserved) |
| 17 | Invalid JSON returns proper error | POST | /api/students | 400, success=false | ✅ PASS |
| 18 | Report generation with empty and populated datasets | GET | /api/reports/* | 200, zero-data safe | ✅ PASS |

### Integration Test Results (Live Server, 2026-10-22)

| Scenario | Outcome |
|---|---|
| Dashboard API response | 10 active students; attendance trend: 85.7% → 83.3% → 83.3% → 100.0% over 4 days |
| Static file serving | HTTP 200 for `GET /` (index.html, correct Content-Type: text/html) |
| Student create / search / filter | Working correctly |
| Attendance record and retrieval | Working correctly |
| Timetable create and collision detection | Working correctly |
| Device toggle (IoT simulation) | Working correctly |
| CSV export | Working correctly (22-line attendance CSV generated) |

### Key Edge Cases Verified

- **Duplicate roll number:** Returns `409 Conflict` without inserting a partial record.
- **Division-by-zero attendance:** Student with zero recorded days returns `0.0%` without crashing.
- **Invalid JSON payload:** Malformed body returns `{success: false, status: 400}` without server crash.
- **Timetable collision:** Overlapping intervals for same room or same teacher both correctly detected.
- **Post-restart persistence:** JSON flat-files correctly reloaded; all IDs and counters resume from last saved state.

---

## J. Results and Screenshots

> **Note:** Static screenshots cannot be embedded in this report as the application is demonstrated live. The evaluator is invited to run the application and observe each view directly. Physical screenshots may be captured and appended during the live demonstration session.

### Frontend View Descriptions

**1. Executive Dashboard (`#dashboard`)**
The dashboard displays four Key Performance Indicator (KPI) cards at the top: total active students, today's attendance count, number of current defaulters (below 75%), and active classroom devices. Below the KPI row, a line chart renders the 4-day attendance trend (85.7% → 83.3% → 83.3% → 100.0% as verified by the live API). The bottom section shows today's lecture schedule and a compact IoT device status panel.

**2. Student Directory (`#students`)**
A searchable, filterable table of all active students with columns for roll number, name, division, department, semester, and status. Action buttons on each row open modal dialogs for editing student details or confirming soft deactivation. A "Add Student" button opens a validated registration form.

**3. Attendance Ledger (`#attendance`)**
A date picker at the top allows selection of any calendar date. The roster for that date is displayed with radio toggles (Present / Absent) for each student. A "Mark All Present" bulk action button is available. Submitting records triggers `POST /api/attendance` with idempotent upsert behaviour.

**4. Lecture Timetable (`#timetable`)**
Seven weekday tabs (Monday through Sunday) display the lecture schedule for the selected day as a time-sorted card list. A "Schedule Lecture" button opens a form; on submission, if a collision is detected, a red inline alert displays the specific conflict (room or teacher). Existing entries have Edit and Delete controls.

**5. Smart Classroom IoT (`#classroom`) [SIMULATED]**
A temperature gauge displays the simulated room temperature in Celsius. An occupancy counter shows the current simulated headcount for classroom CR-302. Below these, individual device cards (with icons for Light, Fan, and Projector) show current power state and intensity level. Toggle switches update device state via `PUT /api/classroom/devices`. A prominent `[SIMULATED]` badge indicates no real hardware is connected.

**6. Reports & Analytics (`#reports`)**
Three report panels: an active student list with computed attendance percentages, a defaulters list (students below 75% threshold), and a full attendance ledger. Two CSV download buttons trigger `GET /api/reports/export-csv?type=attendance` and `?type=students` respectively, downloading well-formed CSV files.

**7. About Project (`#about`)**
A static informational panel describing the project purpose, technology stack, architecture summary, and anticipated viva questions with model answers. Serves as a quick reference during evaluation.

### Verified API Dashboard Response (2026-10-22)

```json
{
  "success": true,
  "status": 200,
  "data": {
    "total_active_students": 10,
    "attendance_trend": [85.7, 83.3, 83.3, 100.0],
    "defaulter_count": 2,
    "classroom": "CR-302",
    "is_simulated": true
  }
}
```

---

## K. Limitations and Future Work

### Current Limitations

1. **IoT is fully simulated:** The most significant limitation of the current implementation is that the "Smart Classroom" IoT module does not interface with any physical hardware. Device states (power, level) are stored in a JSON file and toggled via the API but have no effect on real lights, fans, or projectors. Integration with an Arduino Uno or ESP32 microcontroller via serial communication or MQTT would be required for a production deployment.

2. **No authentication or authorisation:** Any user with network access to port 8080 can read, create, update, or delete any data. There is no login system, no JWT token validation, and no role-based access control (e.g., separating teacher and administrator roles).

3. **Single classroom scope:** The timetable and IoT modules are designed around a single classroom (CR-302). A real deployment would require multi-room support with per-room device inventories and independently managed IoT endpoints.

4. **Linear-scan queries:** The JSON flat-file approach requires scanning entire collections for filtered queries (e.g., attendance by date). For datasets beyond a few thousand records, performance would degrade. An indexed data store would be required at scale.

5. **No real-time push:** The frontend polls the server on page navigation. True real-time updates (e.g., instant attendance notification) would require WebSockets or Server-Sent Events.

### Future Work

| Enhancement | Technology | Priority |
|---|---|---|
| JWT-based authentication and RBAC | C++ JWT library (jwt-cpp) | High |
| Replace JSON flat-files with PostgreSQL | libpqxx C++ client | High |
| Real Arduino/ESP32 IoT integration | MQTT (mosquitto), serial USB | High |
| Multi-classroom support | Schema extension, new API routes | Medium |
| Mobile application | Flutter or React Native | Medium |
| WebSocket real-time updates | cpp-httplib WebSocket support | Low |
| Cloud deployment with Docker | Docker + Nginx reverse proxy | Low |

---

## L. Conclusion

The Smart Classroom Management System successfully achieves all eight stated objectives. A complete, production-quality REST API backend was implemented in C++17, demonstrating advanced object-oriented programming concepts — including encapsulation, abstraction, composition, RAII, templates, and reader-writer synchronisation — within a real, functional application context rather than a contrived exercise.

The system provides digital student lifecycle management, idempotent attendance recording with automated defaulter detection, collision-aware timetable scheduling, simulated IoT device control, and comprehensive reporting with CSV export. All functionality is accessible through a fully responsive Single Page Application served directly from the C++ binary, with zero external npm dependencies.

The comprehensive test suite of 18 scenarios achieved a 100% pass rate on 2026-10-22, covering both happy-path operations and critical edge cases including duplicate detection, division-by-zero safety, invalid JSON handling, and post-restart data persistence. The integration test confirmed live API correctness with real data.

The project is fully prepared for live demonstration. The evaluator can build the project using CMake, launch the server executable, and interact with all seven frontend views in a standard web browser. The codebase is clean, well-structured, and commented, making it suitable for viva questioning on any of the design decisions documented in this report.

---

## M. References

1. **cpp-httplib** — Yuji Hirose. *A C++ header-only HTTP/HTTPS server and client library.* GitHub: https://github.com/yhirose/cpp-httplib

2. **nlohmann/json** — Niels Lohmann. *JSON for Modern C++ (header-only library).* GitHub: https://github.com/nlohmann/json

3. **CMake** — Kitware Inc. *CMake Build System.* https://cmake.org — Version 4.4.4 used.

4. **C++17 Standard** — ISO/IEC JTC1/SC22/WG21. *ISO/IEC 14882:2017 — Programming Language C++.* International Organization for Standardization, 2017.

5. **REST Architectural Style** — Roy Thomas Fielding. *Architectural Styles and the Design of Network-based Software Architectures.* Doctoral Dissertation, University of California, Irvine, 2000. https://www.ics.uci.edu/~fielding/pubs/dissertation/top.htm

6. **Stroustrup, B.** *The C++ Programming Language, 4th Edition.* Addison-Wesley Professional, 2013. ISBN: 978-0321563842.

7. **Meyers, S.** *Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14.* O'Reilly Media, 2014. ISBN: 978-1491903995.

8. **LLVM/Clang** — The LLVM Project. *Clang: a C language family frontend for LLVM.* https://clang.llvm.org — MinGW-w64 UCRT variant, Version 23.1.3 used.

9. **Williams, A.** *C++ Concurrency in Action, 2nd Edition.* Manning Publications, 2019. ISBN: 978-1617294693. *(Reference for `std::shared_mutex` and reader-writer lock patterns.)*

---

*Report prepared as part of the mini-project submission for [PLACEHOLDER: Subject Name], [PLACEHOLDER: Academic Year].*
*Institution: [PLACEHOLDER: College Name], Department of [PLACEHOLDER: Department Name].*
