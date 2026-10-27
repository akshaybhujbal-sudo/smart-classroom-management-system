# Smart Classroom Management System — Viva Q&A Preparation Guide

> Prepared from actual implementation — 35+ questions with model answers

---

## Category 1: Project Overview & Motivation

---

**Q1: Why did you build this project?**

> **A:** Most colleges still manage attendance registers, timetables, and classroom devices using manual methods — paper registers, Excel sheets, and verbal coordination. This is error-prone and time-consuming for both faculty and students. I built this Smart Classroom Management System to digitize and centralize these operations into a single web-accessible application so that attendance tracking, timetable scheduling, and device monitoring can all be managed from a browser without any manual paperwork.

---

**Q2: What problem does it solve?**

> **A:** It solves three main problems simultaneously. First, it automates attendance tracking and instantly flags defaulters — students whose attendance falls below 75% — which would otherwise require manual calculation. Second, it prevents scheduling conflicts by automatically detecting timetable clashes before they are saved. Third, it provides a central record of classroom devices (fans, projectors, lights, etc.) with their simulated on/off status, enabling smarter resource awareness. All three modules communicate through a unified REST API, which means data is always consistent.

---

**Q3: Why did you choose C++ instead of Python or Node.js?**

> **A:** Most web backend projects in this domain default to Python or Node.js, so I deliberately chose C++ to demonstrate its applicability beyond systems programming and to challenge myself. C++ gives us deterministic memory management through RAII and `shared_ptr`, zero-cost abstractions through templates, and native concurrency primitives like `std::shared_mutex`. The `cpp-httplib` and `nlohmann/json` libraries make it perfectly viable for building REST APIs in C++17 without sacrificing safety or performance. This choice also aligns with our curriculum emphasis on C++ as a core language.

---

**Q4: What is the scope of your project?**

> **A:** The project covers four core modules: Student Management (CRUD with soft delete), Attendance Management (mark, view, and compute percentages), Timetable Management (add entries with collision detection), and IoT Device Monitoring (simulated device status toggling). It exposes 20+ REST API endpoints and serves a Single-Page Application frontend from the same C++ server on port 8080. The scope intentionally excludes a production database and real hardware — it uses JSON flat-files for storage and fully software-simulated IoT — making it self-contained and deployable without any external infrastructure. A validated test suite of 18 test cases is also within scope.

---

**Q5: What makes your system "smart"?**

> **A:** The system earns the "smart" label through three intelligent behaviours. First, automatic defaulter detection: the system continuously calculates each student's attendance percentage and flags those below the 75% threshold without any manual intervention. Second, collision-aware scheduling: the timetable module rejects any new entry that would time-overlap with an existing entry for the same room, preventing double-booking automatically. Third, IoT device awareness: the system maintains a real-time status model of classroom devices so administrators can monitor resource usage. These features go beyond passive data storage and provide proactive decision support.

---

## Category 2: C++ Concepts & OOP

---

**Q6: What OOP concepts did you use? Give examples from your code.**

> **A:** I used all four major OOP pillars. **Encapsulation** is demonstrated in every model class — for example, the `Student` struct bundles nine related fields (id, name, roll\_number, department, semester, email, phone, is\_active, created\_at) into a single cohesive unit, and `FileStorageManager` hides all file I/O behind public methods. **Inheritance** is applied through the service layer hierarchy. **Composition** is the relationship between `ReportService` and the storage and student-service objects it holds internally. **Polymorphism** is used through the template-based JSON serialisation (`get<T>()`), which lets us deserialise any field type at compile time. Every class lives inside the `smartclass` namespace to avoid global scope pollution.

---

**Q7: What is RAII? Where exactly did you use it?**

> **A:** RAII stands for Resource Acquisition Is Initialisation — the principle that a resource (memory, file handle, mutex lock) is tied to the lifetime of an object, so it is automatically released when that object goes out of scope. I used RAII in two key places. First, all service and storage objects are managed through `std::shared_ptr`, so heap-allocated objects are automatically destroyed when no more references exist — there are no manual `delete` calls anywhere in the codebase. Second, all mutex locks use `std::shared_lock` and `std::unique_lock` RAII wrappers, ensuring that a lock is always released when the guard object goes out of scope, even if an exception is thrown mid-operation.

---

**Q8: What are templates? Where did you use them?**

> **A:** Templates are a C++ mechanism for writing generic, type-parameterised code that is instantiated at compile time for specific types. I used the `nlohmann/json` library's `get<T>()` template throughout the JSON deserialisation logic. For example, when reading a student record from a JSON file, I call `j.at("name").get<std::string>()` and `j.at("semester").get<int>()` — the same `get` function template works for any type `T`, and the compiler generates the correct specialisation for each. This avoids writing separate parsing functions for strings, integers, and booleans.

---

**Q9: What is `std::shared_mutex` and why did you use it?**

> **A:** `std::shared_mutex` is a C++17 synchronisation primitive that supports two lock modes: shared (read) locks — which multiple threads can hold simultaneously — and exclusive (write) locks — which only one thread can hold at a time and only when no readers are active. I used it in `FileStorageManager` because the HTTP server handles requests on multiple threads concurrently. For read operations like listing all students or fetching attendance records, I acquire a `std::shared_lock`, allowing several requests to read the JSON files in parallel. For write operations like adding a student or marking attendance, I acquire a `std::unique_lock`, which blocks all other readers and writers until the write is complete. This gives maximum concurrency for reads while guaranteeing data consistency for writes.

---

**Q10: What is a `shared_ptr` and why did you use it instead of raw pointers?**

> **A:** `std::shared_ptr` is a C++ smart pointer that maintains a reference count. Each time a new `shared_ptr` is created from the same object, the count increments; when a `shared_ptr` is destroyed, the count decrements; when it reaches zero, the managed object is automatically deleted. I used `shared_ptr` for all service and storage objects because it eliminates the risk of memory leaks and dangling pointers. For example, the `ApiController` holds `shared_ptr<AttendanceService>`, `shared_ptr<StudentService>`, etc. When the controller is destroyed, all services are automatically cleaned up without any explicit `delete`. This is especially safe in a multi-threaded server environment where object lifetimes can be non-trivial.

---

**Q11: What is `namespace smartclass` and why did you define it?**

> **A:** `namespace smartclass` is a custom C++ namespace that encloses all classes, structs, and functions defined in this project. Namespaces prevent name collisions — for example, if `cpp-httplib` or `nlohmann/json` internally define a class named `Student` or `Request`, it would conflict with our own definitions without a namespace. By placing everything under `smartclass`, all our types are unambiguous. It also makes the codebase self-documenting: any symbol prefixed with `smartclass::` is immediately identified as project-defined code rather than a library function.

---

**Q12: How does `FileStorageManager` demonstrate abstraction?**

> **A:** `FileStorageManager` is the sole class responsible for all persistent storage. It exposes a clean public interface — methods like `loadStudents()`, `saveStudents()`, `loadAttendance()`, `saveAttendance()` — and completely hides how data is serialised, where files are located, and how atomic writes are performed. Any service class (like `StudentService` or `AttendanceService`) calls these methods without knowing anything about JSON structure, file paths, or `.tmp`-file rename patterns. This is textbook abstraction: the "what" is public (load/save records) and the "how" is private (file I/O, JSON parsing, mutex locking).

---

**Q13: How does `ReportService` demonstrate composition?**

> **A:** Composition is the OOP design principle of building complex objects by combining simpler ones rather than through inheritance. `ReportService` does not inherit from `StudentService` or `AttendanceService`; instead, it holds them as member objects (via `shared_ptr`). When generating a defaulter report, `ReportService` internally calls `StudentService` to get all active students and `AttendanceService` to compute each student's attendance percentage, then combines the results into a report structure. This "has-a" relationship — `ReportService` has a `StudentService` and has an `AttendanceService` — is composition. It results in loosely coupled code where each service can be tested independently.

---

## Category 3: System Architecture

---

**Q14: Describe your system architecture in layers.**

> **A:** The system follows a three-tier architecture. The **Presentation Layer** is the HTML5/CSS3/JavaScript Single-Page Application served as static files. It renders the UI and makes `fetch()` API calls to the backend. The **Application Layer** consists of the C++ service classes — `StudentService`, `AttendanceService`, `TimetableService`, `DeviceService`, and `ReportService` — which contain all business logic (percentage calculation, collision detection, validation). The **Data Layer** is `FileStorageManager`, which reads from and writes to JSON flat-files on disk using atomic write patterns. `ApiController` sits between the presentation and application layers, routing HTTP requests to the appropriate service method and packaging responses into the standard JSON envelope `{success, status, data, message/error}`.

---

**Q15: What is a REST API? Why did you use it?**

> **A:** REST (Representational State Transfer) is an architectural style for web APIs that uses standard HTTP methods — GET, POST, PUT, DELETE — to perform operations on resources identified by URLs. I used REST because it decouples the frontend from the backend: the browser only needs to know the API endpoints, not how the C++ server works internally. This also makes the API reusable — a mobile app or another service could consume the same endpoints without any changes to the backend. Our 20+ endpoints follow REST conventions, e.g., `GET /api/students` lists all students, `POST /api/students` creates one, `DELETE /api/students/:id` soft-deletes one.

---

**Q16: What is the role of `ApiController`?**

> **A:** `ApiController` is the routing and marshalling layer. It registers all HTTP route handlers with `cpp-httplib`'s server object, so it knows which function to call for each `(method, path)` combination. When a request arrives, `ApiController` parses the HTTP body or URL parameters into C++ objects, calls the appropriate service method, then serialises the returned data back into the standard JSON envelope and writes it to the HTTP response. It also sets CORS headers on every response. `ApiController` contains no business logic itself — it purely translates between the HTTP world and the C++ service world, keeping a clean separation of concerns.

---

**Q17: How does the frontend communicate with the backend?**

> **A:** The frontend uses the browser's native `fetch()` API to send asynchronous HTTP requests to the C++ backend at `http://localhost:8080`. For example, when the user clicks "Add Student," the JavaScript collects the form data, serialises it into a JSON string, and calls `fetch('/api/students', { method: 'POST', body: JSON.stringify(data), headers: { 'Content-Type': 'application/json' } })`. The response is then parsed with `.json()` and the UI updates without a full page reload. All communication is over plain HTTP with JSON payloads — no WebSockets, no polling, no external frameworks.

---

**Q18: What is CORS and why do you set those headers?**

> **A:** CORS stands for Cross-Origin Resource Sharing. Browsers enforce a same-origin policy: a webpage loaded from one origin (e.g., `http://localhost:3000`) cannot make requests to a different origin (e.g., `http://localhost:8080`) unless the server explicitly permits it via CORS headers. Even though our frontend is served from the same C++ server on port 8080, I set `Access-Control-Allow-Origin: *`, `Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS`, and `Access-Control-Allow-Headers: Content-Type` on every response as a defensive measure. This also makes it easy to develop and test the frontend independently (e.g., using a live-server on a different port) without running into blocked requests.

---

## Category 4: Data Storage & Persistence

---

**Q19: Why did you choose JSON flat-files instead of a relational database like MySQL?**

> **A:** For a mini-project scope with a single-server, single-machine deployment, JSON flat-files are pragmatic and self-contained. They require zero external infrastructure — there is no database server to install, configure, or run. The `nlohmann/json` library makes parsing and serialising straightforward in C++. JSON is also human-readable, so the data files can be inspected and debugged directly in a text editor. The trade-offs — no ACID transactions, no indexed queries, O(n) scans — are acceptable at classroom scale where student counts are in the hundreds, not millions. If the system were to scale, replacing `FileStorageManager` with a database-backed implementation would not require changes to any service class, because all storage is abstracted behind that single interface.

---

**Q20: What is the atomic write pattern and why is it important?**

> **A:** The atomic write pattern means: instead of writing directly to the target file, we first write the complete new content to a temporary file (e.g., `students.json.tmp`), and then rename that temporary file over the real file using the OS `rename()` system call. On POSIX systems, `rename()` is atomic — the target file either contains the old content or the new content; there is no intermediate state where the file is partially written. This is critical because if the server crashes or loses power mid-write to the actual file, we could end up with a corrupted, truncated JSON file. With the `.tmp → rename()` pattern, the worst case is that the `.tmp` file is lost but the original remains intact and readable.

---

**Q21: What is thread safety and how did you implement it?**

> **A:** Thread safety means that when multiple threads access shared data concurrently, the data always remains in a consistent state with no race conditions. `cpp-httplib` handles each incoming HTTP request on a separate thread, so multiple requests can arrive simultaneously. I implemented thread safety in `FileStorageManager` using a `std::shared_mutex` with RAII lock guards. Read operations — which are far more frequent — use `std::shared_lock<std::shared_mutex>`, allowing multiple simultaneous readers. Write operations use `std::unique_lock<std::shared_mutex>`, which blocks until all readers have finished and no other writer is active. Because the locks are RAII-managed, they are always released correctly even if an exception occurs during a file operation.

---

**Q22: What is soft deletion? Why not just delete the record?**

> **A:** Soft deletion means marking a record as inactive (`is_active = false`) rather than physically removing it from the data file. Hard deletion permanently erases the record. I chose soft deletion for students because their historical attendance records remain valid and meaningful even after they leave or are removed. If we hard-deleted a student, all their attendance records would become orphaned — referencing a student ID that no longer exists — breaking the integrity of attendance reports. With soft deletion, the student record is preserved for historical reporting purposes, but they no longer appear in active student lists or attendance-marking forms. The `is_active` field can also be set back to `true` if a student is incorrectly deactivated.

---

**Q23: What happens to attendance records when a student is deactivated?**

> **A:** Nothing happens to the existing attendance records — they remain exactly as-is in `attendance.json`. This is by design. The soft delete only sets `is_active = false` on the student record in `students.json`. All historical attendance entries that reference that student's ID are preserved. If a report for a past semester is generated, those records can still be retrieved and the historical attendance percentage can still be computed. The student simply stops appearing in active attendance-marking interfaces. This also means that reactivating a student (setting `is_active = true`) immediately restores their full history without any data recovery operation.

---

## Category 5: Key Algorithms

---

**Q24: How do you calculate attendance percentage? What happens if a student has zero records?**

> **A:** The attendance percentage is calculated as `(attended_classes / total_classes) × 100`, rounded to one decimal place. "Attended classes" counts records where `status == "present"` and "total classes" counts all attendance records for that student. The implementation explicitly guards against division by zero: before performing the division, we check if `total_classes == 0`, and if so, the function returns `0.0` directly without performing the division. This prevents undefined behaviour in C++ (integer division by zero is UB) and gives a sensible, predictable result for students who have no records yet.

---

**Q25: How does your timetable collision detection work? Explain the formula.**

> **A:**  When a new timetable entry is submitted, the system iterates through all existing entries for the same classroom. For each existing entry, it checks the overlap condition: **`max(newStart, existingStart) < min(newEnd, existingEnd)`**. If this condition is true, the two time intervals overlap and the new entry is rejected with an error. This formula is a classic interval-overlap test: two intervals [A, B] and [C, D] overlap if and only if the later of the two start times is strictly before the earlier of the two end times. For example, if a class runs 09:00–10:00 and a new entry is proposed for 09:30–10:30, `max(09:00, 09:30) = 09:30` and `min(10:00, 10:30) = 10:00`, and since `09:30 < 10:00` is true, the collision is detected and the entry is blocked.

---

**Q26: What is idempotent behaviour? How did you implement it in attendance marking?**

> **A:** An operation is idempotent if performing it multiple times has the same result as performing it once. For attendance, the idempotent requirement is: if the same student is marked for the same date more than once (perhaps due to a UI double-submit or a retry), the system should update the existing record rather than create a duplicate. I implemented this by searching the attendance list for an existing record with the same `student_id` and `date` before inserting. If such a record exists, its `status` field is updated in-place and the file is saved. Only if no matching record is found is a new record appended. This prevents attendance records from being double-counted in percentage calculations.

---

**Q27: How do you generate unique IDs for records?**

> **A:** Unique IDs are generated using a combination of the current Unix timestamp and a random component. Specifically, `std::time(nullptr)` gives the current time in seconds, which is combined with `std::rand()` to reduce collision probability even for records created within the same second. The result is formatted as a string. This approach is appropriate for single-server, low-concurrency usage at classroom scale. It is not a UUID standard, but it provides sufficient uniqueness for this project's scope. For a production system, I would use a proper UUID v4 generator or a database-assigned auto-increment key.

---

**Q28: What is the defaulter threshold and how is it calculated?**

> **A:** The defaulter threshold is **75%** — a student whose attendance percentage falls below 75% is classified as a defaulter. This is the standard threshold prescribed by most Indian university regulations for lecture attendance. The calculation: the system retrieves all attendance records for a student, computes `(present_count / total_count) × 100` as described earlier, and compares it to 75.0. If the result is strictly less than 75.0, the student is added to the defaulters list. The `ReportService` generates this list by iterating over all active students and applying this check, returning a sorted collection that the frontend displays in the Defaulters Report section.

---

**Q29: How do you validate input data?**

> **A:** Input validation happens in two places. At the HTTP layer in `ApiController`, the raw request body is parsed and checked: we verify that required fields are present and non-empty (e.g., student name, roll number, department cannot be blank). For timetable entries, the start time is compared to the end time to ensure `startTime < endTime` before the collision check runs. Attendance status is validated to be one of the allowed values (`"present"`, `"absent"`, `"late"`). If any validation fails, the API immediately returns a `400 Bad Request` response with a descriptive error message in the standard JSON envelope, and no file I/O is performed.

---

## Category 6: Frontend & Integration

---

**Q30: What is a Single-Page Application (SPA)?**

> **A:** A Single-Page Application is a web application that loads a single HTML page initially and then dynamically updates the content of that page using JavaScript, rather than loading a brand new HTML page from the server for every user action. In our system, `index.html` is loaded once, and clicking "Students," "Attendance," or "Timetable" in the navigation bar does not trigger a full page reload — instead, JavaScript hides and shows different `<div>` sections and fetches fresh data from the API. This results in a faster, more responsive user experience because only the data changes, not the entire page structure.

---

**Q31: How did you integrate the frontend with the C++ backend?**

> **A:** Integration is achieved through two mechanisms in the `cpp-httplib` server. First, API routes like `GET /api/students` are registered as C++ lambda handlers that execute service logic and return JSON. Second, a catch-all static file handler is registered that serves files from the `frontend/` directory — so a request to `/` returns `index.html`, a request to `/style.css` returns the stylesheet, and so on. The browser loads the HTML/CSS/JS from the same origin (`http://localhost:8080`) and then the JavaScript makes API calls to the same server. There is no separate frontend server, no build step, and no npm — everything is served directly by the C++ process.

---

**Q32: Why did you not use React, Angular, or Vue?**

> **A:** Using a JavaScript framework would require Node.js, npm, and a build pipeline (Webpack, Vite, etc.), which would add significant external tooling complexity to a C++ project. Since one of our goals was to keep the entire stack self-contained and buildable with just CMake and a C++ compiler, plain HTML5 + CSS3 + Vanilla JavaScript was the deliberate and appropriate choice. Vanilla JS with `fetch()`, `async/await`, and the DOM API is entirely sufficient for rendering tables, forms, and dynamic content at the scale of a classroom management system. Using a framework would have been over-engineering for this scope.

---

**Q33: How does the static file serving work in `cpp-httplib`?**

> **A:** `cpp-httplib` provides a `set_mount_point()` method that maps a URL path prefix to a filesystem directory. I configured it to mount the root URL `/` to the `frontend/` directory. When the browser requests any path that does not match a registered API route (e.g., `GET /` or `GET /index.html`), `cpp-httplib` looks up the corresponding file in the `frontend/` directory, reads it, and sends it back with the appropriate `Content-Type` header (e.g., `text/html` for `.html`, `text/css` for `.css`, `application/javascript` for `.js`). This means the C++ server doubles as a static web server with no additional infrastructure required.

---

## Category 7: Testing

---

**Q34: How did you test your application?**

> **A:** I wrote a custom C++ test runner in `tests/test_backend.cpp`. Rather than using an external testing framework like Google Test or Catch2, I implemented a lightweight assertion macro that records pass/fail results and prints a summary at the end. Each test function directly instantiates the service classes and calls their methods, asserting the return values and state changes. The tests run in a self-contained mode using temporary in-memory data, so they do not interfere with any production JSON files. The test binary is built alongside the main server binary using CMake and run separately.

---

**Q35: How many test cases do you have? What do they cover?**

> **A:** There are **18 test cases**, and all 18 pass. They cover the following: student creation and retrieval, student soft-delete verification (confirming `is_active` becomes false and the student count changes correctly), attendance marking and the idempotent update behaviour (marking the same student twice for the same date and verifying only one record exists), attendance percentage calculation including the zero-record edge case, timetable entry creation and collision detection (verifying that an overlapping entry is rejected), device status toggling, and the defaulter threshold boundary (testing a student at exactly 75% and below 75%). Input validation cases — empty name, missing fields — are also included.

---

**Q36: What is the difference between unit testing and integration testing?**

> **A:** **Unit testing** tests a single class or function in complete isolation, replacing all dependencies with stubs or mocks. For example, testing `AttendanceService::calculatePercentage()` with a manually constructed vector of records — no file system involved. **Integration testing** tests how multiple components work together, including real I/O. For example, testing the full flow: `ApiController` receives a POST request → calls `StudentService::addStudent()` → which calls `FileStorageManager::saveStudents()` → and then verifying the file was written correctly. Our test suite in `test_backend.cpp` is primarily unit-level, directly calling service methods, but includes some integration-level checks that verify persistence through the storage layer.

---

**Q37: What edge cases did you test?**

> **A:** The key edge cases tested include: (1) Attendance percentage when `total_classes = 0` — expected result is `0.0` without a division-by-zero crash. (2) Timetable collision with identical time slots — same start and end time for the same room must be rejected. (3) Timetable with adjacent (non-overlapping) slots — e.g., 09:00–10:00 followed by 10:00–11:00 should be allowed since the overlap formula uses a strict less-than comparison. (4) Idempotent attendance — submitting the same student + date combination twice updates the record rather than creating a duplicate. (5) Soft delete — verifying that a deleted student's `is_active` is false but the record still exists in the data file.

---

## Category 8: Limitations & Future Work

---

**Q38: What are the limitations of your system?**

> **A:** There are several honest limitations. First, the storage uses JSON flat-files, which means all data is loaded into memory on every read/write operation — this would not scale to thousands of concurrent users or very large datasets. There is no indexing, so searches are O(n) linear scans. Second, there is no user authentication or authorisation — anyone who can reach port 8080 can perform any operation. Third, the ID generation is time-based and not cryptographically unique — extremely rapid record creation could theoretically produce duplicate IDs. Fourth, the system has no data backup or recovery mechanism beyond the atomic write pattern. Finally, all IoT device control is simulated in software with no actual hardware integration.

---

**Q39: Is the IoT part real? Is any physical hardware connected?**

> **A:** No. The IoT module is **100% simulated in software**. There is no physical hardware — no microcontroller, no Raspberry Pi, no Arduino, no actual sensors or actuators connected to the system. The `ClassroomDevice` model stores device names, types, and a boolean `is_on` status in a JSON file. Toggling a device simply flips the `is_on` flag and saves the file. The purpose of the module is to demonstrate the *concept* of IoT device management and how such a system would be architectured and API-integrated, not to provide actual hardware control. A real implementation would replace the `DeviceService` logic with MQTT pub/sub messages or GPIO calls to actual hardware, while keeping the same API surface.

---

**Q40: How would you improve this system if you had more time?**

> **A:** I would make several improvements in priority order. First, replace JSON flat-files with **SQLite** using the same `FileStorageManager` abstraction interface — this would give us ACID transactions, indexed queries, and significantly better scalability with minimal changes to service code. Second, add **JWT-based authentication** so that only authorised faculty and admins can access the API. Third, implement **real IoT integration** using the MQTT protocol with an ESP32 or Raspberry Pi controlling actual classroom relays. Fourth, add **email/SMS notifications** for students falling below the defaulter threshold. Fifth, add a **proper UUID v4 generator** for ID uniqueness. Sixth, containerise the application using **Docker** so it can be deployed on any machine without manual CMake setup.

---

## Quick Reference Summary

| Category | # Questions |
|---|---|
| Project Overview & Motivation | 5 |
| C++ Concepts & OOP | 8 |
| System Architecture | 5 |
| Data Storage & Persistence | 5 |
| Key Algorithms | 6 |
| Frontend & Integration | 4 |
| Testing | 4 |
| Limitations & Future Work | 3 |
| **Total** | **40** |

---

## Key Numbers to Remember

| Fact | Value |
|---|---|
| Server port | 8080 |
| Total REST endpoints | 20+ |
| Test cases (all passing) | 18 / 18 |
| Defaulter threshold | < 75% |
| Student model fields | 9 |
| AttendanceRecord fields | 7 |
| TimetableEntry fields | 7 |
| ClassroomDevice fields | 6 |
| C++ standard | C++17 |
| Build system | CMake + Clang/MinGW |
| JSON library | nlohmann/json |
| HTTP library | cpp-httplib (httplib.h) |
| Storage format | JSON flat-files |
| IoT hardware | None — 100% simulated |

---

> [!IMPORTANT]
> **IoT Disclaimer — Repeat this if asked:** *"The IoT device module is entirely simulated in software. No physical hardware, microcontroller, sensor, or actuator is connected to this system. The module demonstrates the architectural concept and API design for device management only."*

---

> [!TIP]
> **Presentation tip:** For OOP questions, always name a specific class and field. For algorithm questions, write the formula on the board: `(attended/total) × 100` and `max(s1,s2) < min(e1,e2)`. For testing questions, always say "18 out of 18 test cases pass."

---

*Document generated for internal viva preparation. All answers reflect the actual implementation of the Smart Classroom Management System.*
