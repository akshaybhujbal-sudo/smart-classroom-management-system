# Smart Classroom Management System — Requirements Specification

## 1. Project Overview & Main Objective
The **Smart Classroom Management System** is a unified, locally hosted desktop-web application designed for college engineering mini-project evaluation and demonstration. The system bridges physical classroom administration, student record tracking, attendance automation, daily academic scheduling, and simulated smart IoT classroom environmental monitoring into a coherent, modern platform.

### Core Objectives
1. **Unified Classroom Administration:** Consolidate student information, attendance logs, and academic schedules into a centralized desktop application.
2. **Modern C++ Engineering:** Implement all business logic, data structures, and persistence using modern C++ (C++17) conforming to robust Object-Oriented Programming (OOP) and Clean Architecture principles.
3. **Decoupled Architecture:** Separate the presentation layer (HTML5/CSS3/JavaScript) from the core C++ backend using RESTful JSON APIs.
4. **Interactive IoT Simulation:** Provide simulated environmental telemetry (temperature, occupancy) and device controls (lighting, ventilation, projection) designed to easily accept future Arduino/ESP32 hardware integration.
5. **Academic Rigor:** Produce clean, well-commented code, rigorous input validation, atomic persistence, and clear architectural diagrams suitable for viva defense.

---

## 2. Functional Requirements by Module

### Module A — Executive Dashboard
* **A.1 Summary Metrics:** Provide real-time high-level metric cards:
  - Total registered active students.
  - Today's present student count and absent student count.
  - Today's overall attendance percentage ($$\frac{\text{Present}}{\text{Total Registered}} \times 100\%$$).
* **A.2 Dynamic Timetable Card:** Display today's upcoming and active lectures sorted by start time, indicating subject, faculty, room, and time slot.
* **A.3 Smart Classroom Status Card:** Display live simulated telemetry (temperature in °C, live occupancy count) and active device states (lights, fans, projector).
* **A.4 Recent Activity Log:** Display an audit feed of recent operational actions (e.g., "Student registered", "Attendance saved for 2026-10-21", "Projector powered ON").

### Module B — Student Management
* **B.1 Student Registration:** Capture Roll Number, Full Name, Email, Department, and Semester/Batch.
* **B.2 Uniqueness & Validation:** Enforce strict uniqueness on Roll Number (case-insensitive alphanumeric). Validate email syntax and non-empty name.
* **B.3 Directory & Search:** View all registered students in a tabular format with real-time client-side and backend search filtering by Roll Number or Name.
* **B.4 Profile Modification:** Edit student details (name, email, department, semester) while preserving historical keys.
* **B.5 Safe Deletion (Soft Deactivation):** Implement soft deletion (`is_active: false`) to ensure historical attendance records are never corrupted or orphaned.

### Module C — Attendance Management
* **C.1 Date-Centric Roster:** Allow selection of any academic calendar date (defaults to current date). Render the active student roster for that date.
* **C.2 Status Marking:** Mark each student as either `PRESENT` or `ABSENT` with toggle buttons or bulk actions ("Mark All Present", "Mark All Absent").
* **C.3 Duplicate Prevention:** Ensure only one master attendance record per student per date exists. Submitting again on the same date performs an idempotent update.
* **C.4 Student-Wise Aggregation:** Calculate total classes conducted, total classes attended, and attendance percentage per student across selected date ranges.
* **C.5 Daily Summary Telemetry:** Provide real-time breakdown of present vs. absent counts for the selected date.

### Module D — Timetable Management
* **D.1 Schedule Creation:** Add new timetable slots with Subject Name, Faculty Name, Classroom/Hall ID, Day of Week (`Monday`–`Saturday`), Start Time (`HH:MM`), and End Time (`HH:MM`).
* **D.2 Schedule Validation:** Validate that Start Time precedes End Time ($$\text{StartTime} < \text{EndTime}$$) and detect time-slot collisions for the same classroom or faculty.
* **D.3 Day-Wise Filtering:** Filter and display schedules by day of the week, with quick navigation for "Today's Schedule".
* **D.4 Slot Mutation:** Update slot details or remove cancelled classes.

### Module E — Smart Classroom Monitoring (IoT Simulation)
* **E.1 Environmental Telemetry:**
  - Room Temperature: Simulated ambient temperature with periodic subtle fluctuations (e.g., 22.0°C – 28.5°C).
  - Occupancy Count: Real-time count of people present in the smart classroom, dynamically linked to the latest attendance count or adjustable via simulation controls.
* **E.2 Actuator / Device Control:**
  - Lighting (Room Lights): ON/OFF switch.
  - Ventilation (Smart Ceiling Fans / AC): ON/OFF switch with speed level (Low, Medium, High).
  - Presentation Device (Smart Projector): ON/OFF switch and HDMI/Wireless source indicator.
* **E.3 C++ Backend Synchronization:** All device states and sensor values are managed, validated, and persisted by the C++ backend.
* **E.4 Hardware Extensibility Hook:** Provide clear abstracted C++ hardware interface (`IDeviceController`, `ISensorReader`) so an Arduino Uno or ESP32 serial/HTTP driver can be dropped in without changing business logic.

### Module F — Reports & Analytics
* **F.1 Daily Attendance Ledger:** Complete daily attendance report with summary counts and percentage.
* **F.2 Student Defaulter / Shortage Report:** Filter students with attendance below a designated threshold (e.g., < 75%) for college academic warning lists.
* **F.3 Classroom Utilization Summary:** Overview of classroom operating hours and device runtime hours.
* **F.4 CSV Data Export:** Export attendance reports to standard Comma-Separated Values (`.csv`) for submission to college administration.

---

## 3. Non-Functional Requirements (NFR)

| Category | Requirement | Specification & College Criteria |
| :--- | :--- | :--- |
| **Performance** | Low Latency | API response times under 20ms on `localhost`. Immediate UI updates. |
| **Persistence** | Data Durability | All records saved to clean, human-readable JSON files. Atomic file replacement to prevent corruption on crash. |
| **Reliability** | Fault Tolerance | Backend handles invalid payloads gracefully, returning standard HTTP error codes (`400`, `404`, `409`, `500`) with descriptive JSON error messages. |
| **Security & Privacy** | Local & Fictional | No hardcoded credentials or real PII. Runs exclusively on `localhost` (loopback interface). |
| **Maintainability** | Clean Code | C++17 OOP principles: Single Responsibility, Interface Segregation, RAII, strong typing, clean header/source separation. |
| **Portability** | Multi-Platform Build | Built via standard `CMakeLists.txt` compatible with GCC/MinGW, Clang, or MSVC. |
| **Usability** | Professional UI | Clean modern dashboard with intuitive card layout, clear typography, dark/light contrast, and responsive layout. |

---

## 4. Academic Rubric & Workspace Inspection Report
* **Workspace Status:** The project workspace was inspected upon project initiation. No pre-existing files, rubrics, or institutional PDF guidelines were found.
* **Provisional Standard Applied:** Following standard university guidelines for Bachelor of Engineering / Technology (B.E. / B.Tech / BCA / MCA / B.Sc CS) mini-projects, this document serves as the formal specification.
* **Viva Defense Preparedness:** Every design choice (C++17, REST architecture, soft deletion, atomic file writes) is justified to provide strong talking points during project examination.
