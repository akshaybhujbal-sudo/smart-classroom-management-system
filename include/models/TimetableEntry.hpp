#pragma once

#include <string>
#include "../third_party/nlohmann/json.hpp"

namespace smartclass {

struct TimetableEntry {
    std::string id;
    std::string subject;     // Subject title or code
    std::string teacher;     // Instructor / Faculty name
    std::string classroom;   // Room / Hall identifier
    std::string weekday;     // Weekday (e.g., "MONDAY", "TUESDAY", ...)
    std::string start_time;  // 24-hr format HH:MM (e.g., "09:00")
    std::string end_time;    // 24-hr format HH:MM (e.g., "10:00")

    // JSON Serialization
    friend void to_json(nlohmann::json& j, const TimetableEntry& t) {
        j = nlohmann::json{
            {"id", t.id},
            {"subject", t.subject},
            {"teacher", t.teacher},
            {"classroom", t.classroom},
            {"weekday", t.weekday},
            {"start_time", t.start_time},
            {"end_time", t.end_time}
        };
    }

    // JSON Deserialization
    friend void from_json(const nlohmann::json& j, TimetableEntry& t) {
        if (j.contains("id") && !j["id"].is_null()) t.id = j["id"].get<std::string>();
        if (j.contains("subject") && !j["subject"].is_null()) t.subject = j["subject"].get<std::string>();
        // Backwards compatibility with Part 1 schema subject_code
        else if (j.contains("subject_code") && !j["subject_code"].is_null()) t.subject = j["subject_code"].get<std::string>();

        if (j.contains("teacher") && !j["teacher"].is_null()) t.teacher = j["teacher"].get<std::string>();
        else if (j.contains("faculty_name") && !j["faculty_name"].is_null()) t.teacher = j["faculty_name"].get<std::string>();

        if (j.contains("classroom") && !j["classroom"].is_null()) t.classroom = j["classroom"].get<std::string>();
        else if (j.contains("classroom_id") && !j["classroom_id"].is_null()) t.classroom = j["classroom_id"].get<std::string>();

        if (j.contains("weekday") && !j["weekday"].is_null()) t.weekday = j["weekday"].get<std::string>();
        else if (j.contains("day_of_week") && !j["day_of_week"].is_null()) t.weekday = j["day_of_week"].get<std::string>();

        if (j.contains("start_time") && !j["start_time"].is_null()) t.start_time = j["start_time"].get<std::string>();
        if (j.contains("end_time") && !j["end_time"].is_null()) t.end_time = j["end_time"].get<std::string>();
    }
};

} // namespace smartclass
