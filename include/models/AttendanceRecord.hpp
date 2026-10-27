#pragma once

#include <string>
#include "../third_party/nlohmann/json.hpp"

namespace smartclass {

struct AttendanceRecord {
    std::string id;
    std::string date;         // Format: YYYY-MM-DD
    std::string student_id;   // Student internal ID
    std::string student_roll; // Student roll number (foreign reference)
    std::string status;       // "PRESENT" or "ABSENT"
    std::string recorded_at;  // ISO-8601 Timestamp
    std::string remarks;      // Notes / explanation

    // JSON Serialization
    friend void to_json(nlohmann::json& j, const AttendanceRecord& a) {
        j = nlohmann::json{
            {"id", a.id},
            {"date", a.date},
            {"student_id", a.student_id},
            {"student_roll", a.student_roll},
            {"status", a.status},
            {"recorded_at", a.recorded_at},
            {"remarks", a.remarks}
        };
    }

    // JSON Deserialization
    friend void from_json(const nlohmann::json& j, AttendanceRecord& a) {
        if (j.contains("id") && !j["id"].is_null()) a.id = j["id"].get<std::string>();
        if (j.contains("date") && !j["date"].is_null()) a.date = j["date"].get<std::string>();
        if (j.contains("student_id") && !j["student_id"].is_null()) a.student_id = j["student_id"].get<std::string>();
        if (j.contains("student_roll") && !j["student_roll"].is_null()) a.student_roll = j["student_roll"].get<std::string>();
        if (j.contains("status") && !j["status"].is_null()) a.status = j["status"].get<std::string>();
        if (j.contains("recorded_at") && !j["recorded_at"].is_null()) a.recorded_at = j["recorded_at"].get<std::string>();
        if (j.contains("remarks") && !j["remarks"].is_null()) a.remarks = j["remarks"].get<std::string>();
    }
};

} // namespace smartclass
