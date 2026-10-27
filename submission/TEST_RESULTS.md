# Smart Classroom Management System — Test Results Report

**Execution Date:** 2026-10-22
**Compiler:** Clang 23.1.3 (LLVM-MinGW UCRT)
**Build System:** CMake 4.4.4 with MinGW Makefiles
**Platform:** Windows (x86-64)
**Executable:** `build\smart_classroom_tests.exe`

---

## 1. Build Verification

```
cmake -G "MinGW Makefiles" -B build
cmake --build build -j 4
```

**Result:** ✅ BUILD SUCCESS — Both targets compiled cleanly.
- `build\smart_classroom_server.exe` — Application server binary
- `build\smart_classroom_tests.exe` — Automated test runner binary

*Note: One deprecation warning from cpp-httplib `operator""_t` — harmless, does not affect functionality.*

---

## 2. Automated Backend Unit Tests

### Execution
```
.\build\smart_classroom_tests.exe
```

### Raw Output
```
=================================================================
   SMART CLASSROOM MANAGEMENT SYSTEM — BACKEND AUTOMATED TESTS   
=================================================================
 [PASS] 1. Add a valid student (Generated ID: std-012)
 [PASS] 2. Reject a student with missing required fields (Correctly rejected with 400)
 [PASS] 3. Reject a duplicate roll number (Returned 409 Conflict: A student with roll number 'cs2026-901' already exists.)
 [PASS] 4. Retrieve and search student records (Found matching roll CS2026-901)
 [PASS] 5. Update a student successfully (Updated name & division)
 [PASS] 6. Return an appropriate error for a nonexistent student (Returned 404 Not Found)
 [PASS] 7. Record attendance successfully (Marked PRESENT for 2026-10-21)
 [PASS] 8. Reject duplicate attendance for same student/date (Idempotent update without duplication)
 [PASS] 9. Calculate attendance percentages correctly (50.0% (defaulter) & 0.0% div-zero safe)
 [PASS] 10. Reject an invalid date or attendance status (Rejected invalid date and status)
 [PASS] 11. Create a valid timetable entry (Created slot tt-302 (14:00-15:30))
 [PASS] 12. Reject a conflicting classroom booking (Returned 409 Collision: Collision detected: Classroom 'Lab-101' is already booked on MONDAY from 14:00 to 15:30 for 'CS610: Cloud Computing'.)
 [PASS] 13. Update and delete timetable entries (Updated and deleted successfully)
 [PASS] 14. Update simulated device states (Toggled device power ON with level 2)
 [PASS] 15. Verify saved data remains available after restart (Record loaded from disk after reload)
 [PASS] 16. Deactivating student preserves historical attendance (Soft delete active=false, 2 history logs preserved)
 [PASS] 17. Verify invalid JSON produces proper error response (Response JSON has success=false, status=400)
 [PASS] 18. Report generation with empty and populated datasets (Tested populated and zero-data datasets safely)
=================================================================
   TEST SUMMARY: 18 PASSED, 0 FAILED (TOTAL: 18)
=================================================================
```

### Test Summary Table

| # | Test Scenario | Category | Result |
|---|--------------|----------|--------|
| 1 | Add a valid student | Student CRUD | ✅ PASS |
| 2 | Reject student with missing fields | Validation | ✅ PASS |
| 3 | Reject duplicate roll number | Validation / 409 | ✅ PASS |
| 4 | Retrieve and search student records | Student Query | ✅ PASS |
| 5 | Update a student successfully | Student CRUD | ✅ PASS |
| 6 | Error for nonexistent student | Error Handling / 404 | ✅ PASS |
| 7 | Record attendance successfully | Attendance | ✅ PASS |
| 8 | Reject duplicate attendance (idempotent) | Idempotency | ✅ PASS |
| 9 | Calculate attendance percentages | Algorithm | ✅ PASS |
| 10 | Reject invalid date / status | Validation | ✅ PASS |
| 11 | Create valid timetable entry | Timetable | ✅ PASS |
| 12 | Reject conflicting classroom booking | Collision Detection / 409 | ✅ PASS |
| 13 | Update and delete timetable entries | Timetable CRUD | ✅ PASS |
| 14 | Update simulated device states | IoT Simulation | ✅ PASS |
| 15 | Data persists after restart | Persistence | ✅ PASS |
| 16 | Soft delete preserves attendance history | Soft Delete | ✅ PASS |
| 17 | Invalid JSON → proper error response | Error Handling | ✅ PASS |
| 18 | Report generation (empty + populated) | Reports | ✅ PASS |

**Overall: 18/18 PASSED (100% pass rate)**

---

## 3. Integration Tests — Live Server Verification

**Server:** `.\build\smart_classroom_server.exe --port 8080`
**Method:** PowerShell `Invoke-RestMethod` against running server

| # | Test | Endpoint | Expected | Result |
|---|------|----------|----------|--------|
| 1 | Root page serves HTML | GET / | HTTP 200, text/html | ✅ PASS |
| 2 | Dashboard API | GET /api/dashboard | 200, students.active=10 | ✅ PASS |
| 3 | Create student | POST /api/students | 201, id generated | ✅ PASS |
| 4 | Search student | GET /api/students?search=arjun | 200, found ≥ 1 | ✅ PASS |
| 5 | Record attendance | POST /api/attendance | 200, status=PRESENT | ✅ PASS |
| 6 | Schedule timetable | POST /api/timetable | 201, id=tt-502 | ✅ PASS |
| 7 | Timetable collision | POST /api/timetable (conflicting) | 409 Conflict | ✅ PASS |
| 8 | Toggle device | PUT /api/classroom/devices | 200, is_powered=true | ✅ PASS |
| 9 | Export CSV | GET /api/reports/export-csv | 200, text/csv, 22 lines | ✅ PASS |
| 10 | Cleanup | DELETE /api/students/:id | 200, deactivated | ✅ PASS |

**Overall: 10/10 integration tests passed**

---

## 4. Dashboard API Live Response Summary

```json
{
  "success": true,
  "status": 200,
  "data": {
    "students": { "active": 10, "inactive": 2, "total": 12, "defaulters_count": 2 },
    "today_attendance": { "present": 0, "absent": 0, "percentage": 0.0, "total_logged": 0 },
    "latest_attendance": { "date": "2026-10-25", "present": 1, "absent": 0, "percentage": 100.0 },
    "attendance_trend": [
      { "date": "2026-10-19", "present": 6, "absent": 1, "percentage": 85.7 },
      { "date": "2026-10-20", "present": 5, "absent": 1, "percentage": 83.3 },
      { "date": "2026-10-21", "present": 5, "absent": 1, "percentage": 83.3 },
      { "date": "2026-10-25", "present": 1, "absent": 0, "percentage": 100.0 }
    ],
    "classroom": {
      "classroom_id": "CR-302",
      "classroom_name": "Smart Class & IoT Lab 302",
      "temperature_celsius": 23.8,
      "occupancy_count": 28,
      "is_simulated": true,
      "projector_on": true,
      "lights_on": 2,
      "fans_on": 1,
      "active_devices": 4
    },
    "system_status": "ONLINE"
  }
}
```

---

## 5. Edge Cases Verified

| Edge Case | Mechanism | Verified |
|-----------|-----------|---------|
| Division by zero in attendance % | Returns 0.0 when total=0 | ✅ Test #9 |
| Duplicate roll number | 409 Conflict response | ✅ Test #3 |
| Invalid attendance status (not PRESENT/ABSENT) | 400 Bad Request | ✅ Test #10 |
| Malformed JSON in request body | 400 + error message | ✅ Test #17 |
| Duplicate attendance for same student+date | Idempotent update, no duplication | ✅ Test #8 |
| Timetable overlap same room | 409 Conflict with description | ✅ Test #12 |
| Nonexistent resource lookup | 404 Not Found | ✅ Test #6 |
| Data persistence across process restart | Reload from JSON file | ✅ Test #15 |
| Soft delete preserving history | is_active=false, records intact | ✅ Test #16 |
| Empty dataset report generation | Safe zero-division handling | ✅ Test #18 |

---

*Test results based on actual execution on 2026-10-22. Reproducible by running `.\build\smart_classroom_tests.exe` from the project root.*
