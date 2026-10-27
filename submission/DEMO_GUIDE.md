# Smart Classroom Management System — Live Demonstration Guide

> **Duration:** 5–8 minutes | **Audience:** College evaluator / project guide
> **Prerequisite:** Application server running at `http://127.0.0.1:8080/`

---

## Pre-Demonstration Checklist

Before the evaluator arrives, ensure:

- [ ] C++ server is running: `.\build\smart_classroom_server.exe --port 8080`
- [ ] Browser is open to `http://127.0.0.1:8080/`
- [ ] Browser console (F12) is closed — keep UI clean
- [ ] Demo data is present (check Dashboard shows ≥ 10 active students)
- [ ] Terminal / command prompt is visible (show the C++ server log)

---

## Section 1 — Architecture Walkthrough (1 minute)

> Say: *"Before I demonstrate the live application, let me briefly explain the architecture."*

**Show in terminal / project folder:**
1. Point to `src/` folder — *"This is the C++17 backend — all business logic lives here."*
2. Point to `web/` folder — *"This is the frontend — pure HTML, CSS, and JavaScript."*
3. Point to `data/*.json` files — *"Persistent storage — JSON files written atomically by the C++ server."*
4. Point to `build/smart_classroom_server.exe` — *"This single executable runs the entire backend and serves the frontend on port 8080."*

> **Key point to state:** *"There is no Python, no Node.js, no external runtime. The C++ executable is completely self-contained."*

---

## Section 2 — Executive Dashboard (1 minute)

> Navigate to: `http://127.0.0.1:8080/#dashboard`

**Demo steps:**
1. **Show the 4 KPI cards** — Active Students, Today's Attendance %, Defaulters Count, Temperature
2. **Explain live data** — *"These figures come from a real-time GET /api/dashboard call to the C++ backend."*
3. **Show Today's Schedule panel** — *"Timetable entries for today are retrieved and displayed here."*
4. **Show Device Status panel** — *"The classroom device states (lights, fans, projector) are shown in real-time."*

> **Evaluator question you may get:** *"How often does this update?"*
> **Answer:** *"Each time the page loads or the user navigates to the dashboard, a fresh GET /api/dashboard request is sent. I could add a polling interval but kept it on-demand for the mini-project scope."*

---

## Section 3 — Student Management (1.5 minutes)

> Navigate to: `http://127.0.0.1:8080/#students`

**Demo steps:**

### 3a. Search
1. Type `arjun` in the search box → show matching student
2. Clear search → show all students

### 3b. Add a New Student
1. Click **Register Student**
2. Fill in:
   - Roll No: `CS2026-999`
   - Full Name: `Demo Student`
   - Division: `B`
   - Semester: `6`
   - Email: `demo@college.edu`
3. Click Save — show the green toast notification
4. Confirm new student appears in the list

### 3c. Demonstrate Validation
1. Click **Register Student** again
2. Try to enter the **same roll number** `CS2026-999`
3. Show the **red error toast**: *"409 Conflict — duplicate roll number rejected by the C++ backend"*

### 3d. Soft Delete
1. Find `Demo Student`, click **Deactivate**
2. Confirm → student moves to Archived status
3. Say: *"The student is deactivated — is_active is set to false — but their attendance history remains intact in the JSON file."*

---

## Section 4 — Attendance Management (1 minute)

> Navigate to: `http://127.0.0.1:8080/#attendance`

**Demo steps:**
1. **Select today's date** using the date picker
2. Show the roster of active students with Present/Absent toggles
3. **Mark 2-3 students Present**, **1-2 students Absent**
4. Show the live attendance percentage updating
5. Click **Mark All Present** — show all toggles flip and percentage hits 100%

> **Key point:** *"Every toggle triggers a POST /api/attendance to the C++ backend. The data is immediately persisted to data/attendance.json using an atomic rename operation."*

6. **Navigate away and come back** — show the attendance persists (reloaded from JSON file)

---

## Section 5 — Timetable with Collision Detection (1 minute)

> Navigate to: `http://127.0.0.1:8080/#timetable`

**Demo steps:**
1. Show the weekly timetable grid with existing entries
2. Click **Schedule Lecture Slot**
3. Enter a **valid new slot** (different room, different time) → save → show success
4. Now try to **add a conflicting slot**:
   - Use the same room and same day
   - Set an overlapping time (e.g., if existing is 09:00-10:30, try 10:00-11:00)
5. Show the **red 409 Conflict error** — *"The C++ backend detected the time overlap using interval arithmetic: max(start1,start2) < min(end1,end2)"*

---

## Section 6 — Smart Classroom IoT Panel (45 seconds)

> Navigate to: `http://127.0.0.1:8080/#classroom`

**Demo steps:**
1. Show temperature gauge (~23.8°C), occupancy counter (28)
2. **Toggle the Projector** ON → OFF → ON
3. **Change the Fan speed** level
4. Show the C++ terminal — note the server log entry for the PUT /api/classroom/devices request

> **IMPORTANT — State clearly:** *"This is a software simulation. There is no physical Arduino or ESP32 connected. The device states are stored in data/devices.json and the telemetry values are hardcoded mock data. The architecture is designed so that real IoT hardware could be connected in a production version."*

---

## Section 7 — Reports & CSV Export (30 seconds)

> Navigate to: `http://127.0.0.1:8080/#reports`

**Demo steps:**
1. Show the **Defaulters Report** (students below 75% attendance) highlighted in red/amber
2. Show the **Student Attendance Statistics** table
3. Click **Download Attendance CSV** — show the browser download the CSV file
4. Open the CSV file briefly — *"This is generated in C++ by the CsvExporter utility class."*

---

## Section 8 — Run the Automated Tests (30 seconds)

> Switch to terminal window

```powershell
.\build\smart_classroom_tests.exe
```

Show the evaluator the output:
```
TEST SUMMARY: 18 PASSED, 0 FAILED (TOTAL: 18)
```

> Say: *"These 18 test scenarios cover all business logic — student validation, attendance calculation, collision detection, persistence, error handling — and all pass cleanly."*

---

## Section 9 — Show Source Code (optional, if time permits)

Open the key files to demonstrate C++ quality:

| File | What to show |
|------|-------------|
| `src/services/AttendanceManager.cpp` | Idempotent upsert logic, percentage formula |
| `src/services/TimetableManager.cpp` | Interval overlap collision detection |
| `src/storage/FileStorageManager.cpp` | std::shared_mutex, atomic write with rename |
| `tests/test_backend.cpp` | Test structure and assertions |

---

## Common Evaluator Questions & Quick Answers

| Question | Quick Answer |
|----------|-------------|
| Why C++? | Performance, systems programming education, memory management control |
| Why not a database? | JSON flat-files are sufficient for a mini-project; simpler, zero dependencies |
| Is the IoT real? | No — fully simulated in software. Architecture supports real hardware extension. |
| What is thread safety? | std::shared_mutex allows concurrent reads; exclusive lock for writes |
| How does collision detection work? | max(start1,start2) < min(end1,end2) ⇒ time overlap |
| What is soft deletion? | is_active=false preserves history; not actually removed from JSON |
| How many endpoints? | 20+ REST endpoints across 6 resource groups |
| What is idempotent? | Same attendance request for same student+date updates, never duplicates |

---

## Demonstration Wrap-Up (15 seconds)

> Say: *"To summarize — this is a fully functional Smart Classroom Management System built entirely in C++17, with a responsive single-page web frontend, 20+ REST API endpoints, persistent JSON storage, automated test coverage, and simulated IoT classroom monitoring. The project demonstrates real-world C++ software architecture including OOP design patterns, thread safety, REST API design, and test-driven verification."*

---

*End of demonstration guide.*
