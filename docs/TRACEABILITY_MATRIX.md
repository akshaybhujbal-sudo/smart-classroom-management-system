# Requirements Traceability Matrix (RTM)

The Requirements Traceability Matrix maps every mandatory functional and non-functional requirement to its responsible subsystem module, planned implementation stage across the 5-part project roadmap, and the concrete verification evidence required to satisfy college evaluation rubrics.

| Req ID | Requirement Description | Subsystem Module | Stage | Implementation & Test Status | Verification Evidence |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **REQ-A.1** | Real-time high-level metric cards (Total students, present, absent, %) | Module A (Dashboard) | Part 2 & Part 3 | **Complete & Verified** | Live dashboard renders 4 KPI metric cards populated dynamically via `API.getDashboard()`. |
| **REQ-A.2** | Today's timetable schedule card on dashboard | Module A (Dashboard) | Part 2 & Part 3 | **Complete & Verified** | Automatically detects current weekday and populates lecture slots from `API.getTimetable()`. |
| **REQ-A.3** | Live telemetry (temperature, occupancy) and device status | Module A (Dashboard) | Part 2 & Part 3 | **Complete & Verified** | Displays room ambient temp, live occupancy count, and lighting/projector states. |
| **REQ-A.4** | Recent activities / audit feed & status indicators | Module A (Dashboard) | Part 2 & Part 3 | **Complete & Verified** | Dynamic C++ Engine online/offline badge, live digital clock, and sync action. |
| **REQ-B.1** | Register new student record with full details | Module B (Student) | Part 2 & Part 3 | **Complete & Verified** | Register Student modal with client + server validation, ID assignment, and toast alert. |
| **REQ-B.2** | Enforce unique roll number and input validation | Module B (Student) | Part 2 & Part 3 | **Complete & Verified** | C++ backend rejects duplicate roll with 409 Conflict; UI shows descriptive error toast. |
| **REQ-B.3** | Student directory view with live search by roll number/name | Module B (Student) | Part 2 & Part 3 | **Complete & Verified** | Real-time debounced search bar and status filter (`All`, `Active`, `Archived`). |
| **REQ-B.4** | Edit student profile details | Module B (Student) | Part 2 & Part 3 | **Complete & Verified** | Edit Student modal updates full name, division, department, email, semester. |
| **REQ-B.5** | Safe deletion via soft-deactivation | Module B (Student) | Part 2 & Part 3 | **Complete & Verified** | Confirmation modal sets `is_active=false`; historical logs remain intact. |
| **REQ-C.1** | Calendar date selector for attendance | Module C (Attendance) | Part 2 & Part 3 | **Complete & Verified** | Interactive `<input type="date">` loads records for selected calendar date dynamically. |
| **REQ-C.2** | Present / Absent status marking with batch controls | Module C (Attendance) | Part 2 & Part 3 | **Complete & Verified** | Radio toggles per student, plus "Mark All Present" & "Mark All Absent" shortcuts. |
| **REQ-C.3** | Duplicate prevention for same student and date | Module C (Attendance) | Part 2 & Part 3 | **Complete & Verified** | Idempotent upsert: saving updates status without creating duplicate database rows. |
| **REQ-C.4** | Student-wise attendance percentage calculation | Module C (Attendance) | Part 2 & Part 3 | **Complete & Verified** | Computed via C++ backend; 0 recorded sessions safely returns 0.0% without division-by-zero. |
| **REQ-C.5** | Daily attendance summary metrics | Module C (Attendance) | Part 2 & Part 3 | **Complete & Verified** | Strip renders total present, absent, and daily percentage rate for active date. |
| **REQ-D.1** | Add, view, edit and delete timetable entries | Module D (Timetable) | Part 2 & Part 3 | **Complete & Verified** | Weekly schedule view with full modal CRUD actions and confirmation dialogs. |
| **REQ-D.2** | Timetable validation and collision detection | Module D (Timetable) | Part 2 & Part 3 | **Complete & Verified** | Start < End time checked; room/faculty overlap rejects with 409 Conflict banner toast. |
| **REQ-D.3** | Filter timetable by day of week | Module D (Timetable) | Part 2 & Part 3 | **Complete & Verified** | Day tabs (`ALL`, `MONDAY`, `TUESDAY`, `WEDNESDAY`, `THURSDAY`, `FRIDAY`, `SATURDAY`). |
| **REQ-E.1** | Real-time classroom temperature & occupancy telemetry | Module E (Classroom IoT) | Part 2 & Part 3 | **Complete & Verified** | Telemetry cards render temperature in °C and live student occupancy count. |
| **REQ-E.2** | Controls for simulated lights, fans, and projector | Module E (Classroom IoT) | Part 2 & Part 3 | **Complete & Verified** | Interactive toggle switches for appliances with optimistic updates and C++ rollback. |
| **REQ-E.3** | Hardware abstraction & simulation disclaimer | Module E (Classroom IoT) | Part 2 & Part 3 | **Complete & Verified** | Prominent "SIMULATION MODE" banner clarifying simulated sensors for college viva. |
| **REQ-F.1** | Comprehensive daily attendance report | Module F (Reports) | Part 2 & Part 3 | **Complete & Verified** | Summary KPI cards and breakdown for academic administrative review. |
| **REQ-F.2** | Student-wise aggregate & defaulter report (<75%) | Module F (Reports) | Part 2 & Part 3 | **Complete & Verified** | Students failing the 75% threshold are flagged with `<span class="badge defaulter">`. |
| **REQ-F.3** | Classroom device utilization report | Module F (Reports) | Part 2 & Part 3 | **Complete & Verified** | Dashboard and classroom views display active vs total appliance counts. |
| **REQ-F.4** | Export attendance records to CSV format | Module F (Reports) | Part 2 & Part 3 | **Complete & Verified** | One-click "Export Attendance CSV" and "Export Students CSV" buttons stream `.csv`. |
| **NFR-1** | C++17 modular OOP backend | Architecture | Part 2 & Part 3 | **Complete & Verified** | Clean separation of concerns between storage, services, controllers, and web UI. |
| **NFR-2** | Persistent local storage via JSON | Data Layer | Part 2 & Part 3 | **Complete & Verified** | Data changes persist to disk atomically and survive full server restart. |
| **NFR-3** | Browser UI with zero npm dependencies | Frontend | Part 3 | **Complete & Verified** | Pure HTML5, modern CSS3 with CSS variables, and native ES6 JavaScript. |
| **NFR-4** | CMake build configuration | Build System | Part 2 & Part 3 | **Complete & Verified** | Single `cmake --build build` generates standalone binaries. |
| **NFR-5** | Safe, fictional sample data for demo | Data Layer | Part 2 & Part 3 | **Complete & Verified** | Pre-populated with realistic fictional students, timetable slots, and attendance logs. |

| **NFR-6** | Automated test suite with ≥15 test scenarios | Testing | Part 4 | **Complete & Verified** | 18 test scenarios in `tests/test_backend.cpp` — 18/18 PASS as of 2026-10-22. |
| **NFR-7** | Integration of all modules in a live working demo | Integration | Part 4 | **Complete & Verified** | 10/10 integration tests pass against live server; all 7 UI views functional. |
| **NFR-8** | Comprehensive submission documentation package | Documentation | Part 5 | **Complete** | `submission/` folder: PROJECT_REPORT.md, PRESENTATION.md, DEMO_GUIDE.md, VIVA_QA.md (40 Q&A), TEST_RESULTS.md, QUICK_REFERENCE.md, SUBMISSION_CHECKLIST.md. |
| **NFR-9** | Dashboard attendance trend analytics | Dashboard | Part 4 | **Complete & Verified** | `GET /api/dashboard` returns `attendance_trend[]` with date, present, absent, percentage per day. |
| **NFR-10** | Thread-safe concurrent request handling | Architecture | Part 2 | **Complete & Verified** | `std::shared_mutex` in `FileStorageManager` with shared reads and exclusive writes. |
