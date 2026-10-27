# Smart Classroom Management System — REST API Specification

This document details the RESTful HTTP API provided by the C++ backend for the **Smart Classroom Management System**. All endpoints communicate using standard JSON payloads over HTTP.

- **Base URL:** `http://127.0.0.1:8080` (or configured port)
- **Content-Type:** `application/json` (except CSV export: `text/csv`)
- **Character Encoding:** UTF-8
- **CORS Support:** Full pre-flight `OPTIONS` handling enabled for cross-origin development.

---

## 1. Response Envelope Format

### Successful Response
All successful responses return an HTTP `200 OK` or `201 Created` status with the following structure:
```json
{
  "success": true,
  "status": 200,
  "data": { ... },
  "message": "Optional operational feedback"
}
```

### Error Response
Errors return standard HTTP status codes (`400 Bad Request`, `404 Not Found`, `409 Conflict`, `500 Internal Error`):
```json
{
  "success": false,
  "status": 409,
  "error": "Human-readable description of error or collision"
}
```

---

## 2. Executive Dashboard Endpoints

### `GET /api/dashboard`
Returns live summary totals aggregated directly from active backend records.

#### Example Response:
```json
{
  "success": true,
  "status": 200,
  "data": {
    "system_status": "ONLINE",
    "timestamp": "2026-10-21T18:08:09Z",
    "current_date": "2026-10-21",
    "students": {
      "total": 11,
      "active": 10,
      "inactive": 1,
      "defaulters_count": 2
    },
    "today_attendance": {
      "total_logged": 6,
      "present": 5,
      "absent": 1,
      "percentage": 83.3
    },
    "classroom": {
      "classroom_id": "CR-302",
      "classroom_name": "Smart Class & IoT Lab 302",
      "temperature_celsius": 23.8,
      "occupancy_count": 28,
      "is_simulated": true,
      "total_devices": 4,
      "active_devices": 3,
      "lights_on": 2,
      "fans_on": 1,
      "projector_on": false,
      "last_sync": "2026-10-21T09:45:00Z"
    }
  }
}
```

---

## 3. Student Management Endpoints

### `GET /api/students`
Retrieves all registered students. Supports optional query parameters.
- **Query Parameters:**
  - `search` *(optional)*: Substring filter matching roll number, name, division, or department.
  - `include_inactive` *(optional, default `true`)*: Set `false` to retrieve only active students.

### `GET /api/students/:id`
Retrieves a single student record by their internal ID (e.g., `std-001`).
- **Response:** `200 OK` with student object, or `404 Not Found`.

### `POST /api/students`
Registers a new student with validation and roll number uniqueness check.
#### Request Body:
```json
{
  "roll_number": "CS2026-042",
  "full_name": "Kavya Sundaram",
  "division": "A",
  "department": "Computer Science & Engineering",
  "email": "kavya.sundaram@college.edu",
  "semester": 6
}
```
- **Responses:**
  - `201 Created`: Student registered with generated ID and timestamp.
  - `400 Bad Request`: Missing required fields, invalid email format, or invalid semester.
  - `409 Conflict`: A student with the submitted roll number already exists.

### `PUT /api/students/:id`
Updates an existing student's details.
- **Responses:** `200 OK` on success, `404 Not Found` if ID does not exist, `409 Conflict` if new roll number belongs to another student.

### `DELETE /api/students/:id`
Performs a safe **soft deactivation** (`is_active = false`). The student is removed from upcoming rosters while preserving historical attendance integrity.
- **Response:** `200 OK`.

---

## 4. Attendance Management Endpoints

### `GET /api/attendance`
Retrieves attendance records filtered by date or student.
- **Query Parameters:**
  - `date=YYYY-MM-DD`: Returns all records for the specified calendar date.
  - `student=std-001`: Returns historical records for the specified student.
  - *(Omitted)*: Defaults to today's date.

### `GET /api/attendance/student/:id`
Retrieves full attendance history logs for a specific student.

### `POST /api/attendance`
Records or idempotently updates attendance for a student on a specific date.
#### Request Body:
```json
{
  "date": "2026-10-21",
  "student_id": "std-001",
  "student_roll": "CS2026-001",
  "status": "PRESENT",
  "remarks": "On time"
}
```
- **Business Rules:**
  - Rejects invalid dates or statuses (`400 Bad Request`).
  - Rejects attendance for deactivated/inactive students (`400 Bad Request`).
  - If a record already exists for the student on that date, updates status idempotently without duplicating the row.
- **Responses:** `200 OK`.

### `PUT /api/attendance/:id`
Updates status or remarks for an existing attendance record.
- **Responses:** `200 OK` on success, `404 Not Found` if record does not exist.

---

## 5. Timetable Management Endpoints

### `GET /api/timetable`
Retrieves scheduled lecture entries.
- **Query Parameters:**
  - `day=MONDAY`: Filters entries for a specific day of the week.
  - *(Omitted)*: Returns all weekly entries sorted chronologically.

### `POST /api/timetable`
Schedules a new lecture slot with automatic collision detection.
#### Request Body:
```json
{
  "subject": "CS604: Internet of Things",
  "teacher": "Prof. S. Joshi",
  "classroom": "CR-302 (Smart Hall)",
  "weekday": "WEDNESDAY",
  "start_time": "09:30",
  "end_time": "10:30"
}
```
- **Validation & Collision Rules:**
  - Validates `end_time > start_time`.
  - Collision occurs if another lecture occupies the **same classroom** or the **same instructor** on the same weekday with overlapping intervals ($$\max(S_1, S_2) < \min(E_1, E_2)$$).
- **Responses:**
  - `201 Created`: Successfully scheduled.
  - `400 Bad Request`: Invalid time range or missing fields.
  - `409 Conflict`: Conflicting classroom or teacher booking.

### `PUT /api/timetable/:id`
Updates an existing lecture slot with collision re-evaluation.

### `DELETE /api/timetable/:id`
Deletes a scheduled slot by ID.

---

## 6. Smart Classroom IoT Endpoints

### `GET /api/classroom`
Returns live simulated telemetry and hardware device states.

### `PUT /api/classroom/devices` & `POST /api/classroom/devices`
Updates the operational state of a simulated appliance (light, fan, projector).
#### Request Body:
```json
{
  "device_id": "dev-proj-01",
  "is_powered": true,
  "level": 1
}
```
- **Response:** `200 OK` with updated `ClassroomState`.

---

## 7. Reports & Analytics Endpoints

### `GET /api/reports/attendance?date=YYYY-MM-DD`
Returns a summary report for a given date including total, present, absent counts, and daily percentage.

### `GET /api/reports/students`
Returns student-wise aggregate statistics (total sessions, attended sessions, attendance percentage, and defaulter status).

### `GET /api/reports/defaulters`
Returns all students whose attendance falls below the institutional 75% threshold ($$\text{Attendance} < 75\%$$).

### `GET /api/reports/export-csv`
Streams a standard CSV file directly to the client browser for spreadsheet analysis.
- **Query Parameters:**
  - `type=attendance` *(default)*: Generates `attendance_report.csv`.
  - `type=students`: Generates `students_report.csv`.
  - `date=YYYY-MM-DD` *(optional)*: Filters attendance records for a specific date.
- **Response Header:** `Content-Type: text/csv`
