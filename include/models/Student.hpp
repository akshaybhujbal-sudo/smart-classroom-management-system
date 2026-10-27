#pragma once

#include <string>
#include "../third_party/nlohmann/json.hpp"

namespace smartclass {

struct Student {
    std::string id;
    std::string roll_number;
    std::string full_name;
    std::string division{"A"};
    std::string department{"Computer Science & Engineering"};
    std::string email;
    int semester{1};
    bool is_active{true};
    std::string created_at;

    // JSON Serialization
    friend void to_json(nlohmann::json& j, const Student& s) {
        j = nlohmann::json{
            {"id", s.id},
            {"roll_number", s.roll_number},
            {"full_name", s.full_name},
            {"division", s.division},
            {"department", s.department},
            {"email", s.email},
            {"semester", s.semester},
            {"is_active", s.is_active},
            {"created_at", s.created_at}
        };
    }

    // JSON Deserialization
    friend void from_json(const nlohmann::json& j, Student& s) {
        if (j.contains("id") && !j["id"].is_null()) s.id = j["id"].get<std::string>();
        if (j.contains("roll_number") && !j["roll_number"].is_null()) s.roll_number = j["roll_number"].get<std::string>();
        if (j.contains("full_name") && !j["full_name"].is_null()) s.full_name = j["full_name"].get<std::string>();
        if (j.contains("division") && !j["division"].is_null()) s.division = j["division"].get<std::string>();
        if (j.contains("department") && !j["department"].is_null()) s.department = j["department"].get<std::string>();
        if (j.contains("email") && !j["email"].is_null()) s.email = j["email"].get<std::string>();
        if (j.contains("semester") && !j["semester"].is_null()) s.semester = j["semester"].get<int>();
        if (j.contains("is_active") && !j["is_active"].is_null()) s.is_active = j["is_active"].get<bool>();
        if (j.contains("created_at") && !j["created_at"].is_null()) s.created_at = j["created_at"].get<std::string>();
    }
};

} // namespace smartclass
