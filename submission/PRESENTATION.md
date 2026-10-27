# Smart Classroom Management System — Presentation Slides

> **Format:** 13 slides | **Duration:** 8–10 minutes
> Convert this Markdown to PowerPoint using your institution's template, or present directly from a browser-rendered Markdown viewer.

---

## SLIDE 1 — Title Slide

# Smart Classroom Management System

**A C++17 REST API Backend with Browser-Based Frontend**

Mini-Project Presentation

---

**[PLACEHOLDER: Student Name]** | Roll No: [PLACEHOLDER] | Div: [PLACEHOLDER]
**Department of** [PLACEHOLDER: Department]
**[PLACEHOLDER: College Name]**

**Academic Year:** [PLACEHOLDER] | **Semester:** [PLACEHOLDER]
**Guide:** [PLACEHOLDER: Guide Name, Designation]

---

*Presenter note: Start with a confident introduction. "Today I will demonstrate a fully working Smart Classroom Management System built entirely in C++17."*

---

## SLIDE 2 — Problem Statement

### The Problem with Traditional Classroom Management

| Traditional System | Issues |
|-------------------|--------|
| Paper attendance registers | Error-prone, easy to forge, slow to analyze |
| Physical timetable boards | No collision detection, manual updates |
| Disconnected tools | No unified dashboard view |
| No IoT integration | No environmental monitoring |

**Our Solution:** A unified, web-accessible classroom management system with C++ performance at its core.

---

*Presenter note: Briefly describe each pain point. Emphasize that this system solves all four.*

---

## SLIDE 3 — Project Objectives

### What We Built

✅ Unified executive **dashboard** with live statistics
✅ Full student **CRUD** with validation and soft deletion
✅ Digital **attendance** ledger with defaulter detection
✅ Weekly **timetable** with automatic collision detection
✅ Simulated **IoT** classroom environmental monitoring
✅ **Reports** and CSV data export
✅ **REST API** backend in C++17 — production-grade architecture
✅ **18 automated tests** — all passing

---

## SLIDE 4 — Technology Stack

### Technologies Used

| Layer | Technology | Reason |
|-------|-----------|--------|
| **Backend** | C++17 | Performance, systems programming |
| **HTTP Server** | cpp-httplib (header-only) | Zero dependencies, fast |
| **JSON** | nlohmann/json (header-only) | Type-safe, idiomatic C++ |
| **Build** | CMake 3.16+ | Cross-platform, industry standard |
| **Frontend** | HTML5 + CSS3 + Vanilla JS | No framework overhead |
| **Storage** | JSON flat-files | Simple, self-contained, portable |

> **Zero external runtime dependencies.** The compiled `.exe` is self-contained.

---

*Presenter note: Emphasize the C++ choice. "A Python or Node backend could have been used, but C++ demonstrates real systems programming skills."*

---

## SLIDE 5 — System Architecture

### Layered Architecture

```
┌─────────────────────────────────────────┐
│   PRESENTATION LAYER                    │
│   web/index.html + CSS + JavaScript     │
│   (Single-Page Application, 7 views)    │
└──────────────┬──────────────────────────┘
               │ REST HTTP JSON (port 8080)
┌──────────────▼──────────────────────────┐
│   CONTROLLER LAYER                      │
│   ApiController — 20+ REST endpoints    │
│   CORS headers, JSON envelope, routing  │
├─────────────────────────────────────────┤
│   SERVICE LAYER                         │
│   StudentManager │ AttendanceManager    │
│   TimetableManager │ ClassroomManager   │
│   ReportService                         │
├─────────────────────────────────────────┤
│   STORAGE LAYER                         │
│   FileStorageManager                    │
│   std::shared_mutex │ Atomic writes     │
├─────────────────────────────────────────┤
│   DATA LAYER                            │
│   data/students.json                    │
│   data/attendance.json                  │
│   data/timetable.json                   │
│   data/devices.json                     │
└─────────────────────────────────────────┘
```

---

## SLIDE 6 — C++ OOP Concepts Demonstrated

### Object-Oriented Programming in Action

| Concept | Implementation |
|---------|---------------|
| **Encapsulation** | Each service class exposes only a public interface |
| **Composition** | `ReportService` composes 4 other service objects |
| **RAII** | `std::shared_ptr<>` for automatic memory management |
| **Templates** | `json.get<Student>()` — type-safe deserialization |
| **Abstraction** | `FileStorageManager` hides all JSON I/O details |
| **Namespaces** | `namespace smartclass` avoids symbol collisions |
| **Lambdas** | HTTP route handlers defined as inline lambdas |

**C++17 features used:** `std::filesystem`, `std::shared_mutex`, structured bindings, `std::optional`

---

## SLIDE 7 — Data Models

### 4 Core Domain Entities

**Student**
`id | roll_number | full_name | division | department | semester | email | is_active`

**AttendanceRecord**
`id | date (YYYY-MM-DD) | student_id | status (PRESENT/ABSENT) | recorded_at | remarks`

**TimetableEntry**
`id | subject | teacher | classroom | weekday | start_time | end_time`

**ClassroomDevice**
`id | device_type (LIGHT/FAN/PROJECTOR) | name | is_powered | level (0–3) | location`

> **Soft Deletion:** Students are never hard-deleted. `is_active = false` preserves all attendance history.

---

## SLIDE 8 — Key Algorithms

### Business Logic Highlights

**1. Attendance Percentage Calculation**
```
percentage = (attended / total) × 100
→ Division-by-zero safe: returns 0.0 when total = 0
→ Defaulter threshold: percentage < 75.0
```

**2. Timetable Collision Detection**
```
Overlap = max(start1, start2) < min(end1, end2)
→ Checked per classroom AND per teacher
→ Returns 409 Conflict with descriptive message
```

**3. Atomic JSON Write**
```
Write to file.json.tmp
→ std::filesystem::rename(tmp, target)
→ Rename is atomic on POSIX/NTFS — no corrupt reads
```

**4. Idempotent Attendance**
```
If record(student_id, date) exists → UPDATE
Else → INSERT
→ No duplicate entries ever created
```

---

## SLIDE 9 — REST API Design

### 20+ REST Endpoints

| Group | Endpoints |
|-------|----------|
| Dashboard | `GET /api/dashboard` |
| Students | `GET/POST /api/students`, `GET/PUT/DELETE /api/students/:id` |
| Attendance | `GET/POST /api/attendance`, `PUT /api/attendance/:id` |
| Timetable | `GET/POST /api/timetable`, `PUT/DELETE /api/timetable/:id` |
| Classroom | `GET /api/classroom`, `PUT/POST /api/classroom/devices` |
| Reports | `GET /api/reports/students`, `/defaulters`, `/export-csv` |
| Static | `GET /` → serves web/index.html |

**Response envelope:**
```json
{ "success": true, "status": 200, "data": { ... }, "message": "..." }
```

---

## SLIDE 10 — Frontend UI Overview

### 7 Application Views

| View | Key Feature |
|------|------------|
| **Dashboard** | 4 KPI cards, attendance trend, device status |
| **Students** | Search, filter, Register modal, soft deactivation |
| **Attendance** | Date picker, roster, Mark All shortcut, % indicator |
| **Timetable** | Weekday tabs, Schedule modal, 409 collision alerts |
| **Smart Classroom** | Temperature, occupancy, device toggle switches |
| **Reports** | Defaulters list, attendance ledger, CSV download |
| **About** | Project brief, architecture, viva Q&A |

**Zero npm. Zero frameworks. Pure HTML/CSS/JavaScript.**
Frontend served directly by the C++ backend on port 8080.

---

## SLIDE 11 — Testing

### 18 Automated Test Scenarios — All Pass

```
=================================================================
   TEST SUMMARY: 18 PASSED, 0 FAILED (TOTAL: 18)
=================================================================
```

**Test Coverage:**

| Area | Tests |
|------|-------|
| Student CRUD & validation | Tests 1–6 |
| Attendance logic & calculation | Tests 7–10 |
| Timetable & collision detection | Tests 11–13 |
| IoT device state management | Test 14 |
| Data persistence across restart | Test 15 |
| Soft delete & history preservation | Test 16 |
| Error handling (invalid JSON, 404) | Tests 17–18 |

---

## SLIDE 12 — Limitations & Future Work

### Current Limitations

| Limitation | Note |
|-----------|------|
| IoT is **simulated** | No physical hardware connected — software mock only |
| No authentication | No login/logout — single user assumed |
| Single classroom | Fixed to classroom CR-302 in current version |
| HTTP only | No HTTPS — suitable for local LAN demonstration |

### Future Enhancements

- JWT authentication and role-based access (Admin / Teacher / Student)
- PostgreSQL or SQLite database backend
- Real Arduino/ESP32 hardware integration via serial/MQTT
- Mobile-responsive PWA for attendance on smartphones
- Automated email alerts for defaulters

---

## SLIDE 13 — Conclusion & Demo

### Summary

✅ **Fully working** C++17 backend + browser frontend
✅ **20+ REST endpoints** with proper error handling
✅ **18/18 automated tests passing**
✅ **4 core modules** — Students, Attendance, Timetable, IoT
✅ **Real-time dashboard** with live data aggregation
✅ **CSV export** for institutional data sharing
✅ **Professional code quality** — RAII, thread safety, atomic writes

---

### 🎯 Live Demonstration

> "Let me now open the application and walk through the key features."

**URL:** `http://127.0.0.1:8080/`

---

*Thank you! Questions?*

---

*End of presentation. See DEMO_GUIDE.md for the step-by-step demonstration walkthrough.*
