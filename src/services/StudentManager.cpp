#include "../../include/services/StudentManager.hpp"
#include "../../include/utils/Validation.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace smartclass {

StudentManager::StudentManager(std::shared_ptr<FileStorageManager> storage)
    : storage_(std::move(storage)) {}

std::string StudentManager::generateStudentId(const std::vector<Student>& existing) {
    int max_id = 0;
    for (const auto& s : existing) {
        if (s.id.rfind("std-", 0) == 0) {
            try {
                int val = std::stoi(s.id.substr(4));
                if (val > max_id) max_id = val;
            } catch (...) {}
        }
    }
    std::ostringstream ss;
    ss << "std-" << std::setw(3) << std::setfill('0') << (max_id + 1);
    return ss.str();
}

ServiceResult<void> StudentManager::validateStudentFields(const Student& s, bool /*is_update*/) {
    std::string roll = utils::trim(s.roll_number);
    std::string name = utils::trim(s.full_name);
    std::string email = utils::trim(s.email);

    if (roll.empty()) {
        return ServiceResult<void>::fail("Roll number is required and cannot be empty.", 400);
    }
    if (roll.length() < 3) {
        return ServiceResult<void>::fail("Roll number must be at least 3 characters long.", 400);
    }
    if (name.empty()) {
        return ServiceResult<void>::fail("Student full name is required and cannot be empty.", 400);
    }
    if (name.length() < 2) {
        return ServiceResult<void>::fail("Student name must be at least 2 characters long.", 400);
    }
    if (!email.empty() && !utils::isValidEmail(email)) {
        return ServiceResult<void>::fail("Invalid email address format.", 400);
    }
    if (s.semester < 1 || s.semester > 8) {
        return ServiceResult<void>::fail("Semester must be between 1 and 8.", 400);
    }

    return ServiceResult<void>::ok();
}

ServiceResult<Student> StudentManager::addStudent(Student student) {
    // Trim string inputs
    student.roll_number = utils::trim(student.roll_number);
    student.full_name = utils::trim(student.full_name);
    student.division = utils::trim(student.division);
    student.department = utils::trim(student.department);
    student.email = utils::trim(student.email);

    if (student.division.empty()) student.division = "A";
    if (student.department.empty()) student.department = "Computer Science & Engineering";

    auto val_res = validateStudentFields(student, false);
    if (!val_res.success) {
        return ServiceResult<Student>::fail(val_res.error_message, val_res.http_status);
    }

    auto students = storage_->loadStudents();

    // Check duplicate roll number (case-insensitive)
    std::string new_roll_upper = utils::toUpper(student.roll_number);
    for (const auto& existing : students) {
        if (utils::toUpper(existing.roll_number) == new_roll_upper) {
            return ServiceResult<Student>::fail(
                "A student with roll number '" + student.roll_number + "' already exists.", 409);
        }
    }

    // Set identifiers and metadata
    if (student.id.empty()) {
        student.id = generateStudentId(students);
    }
    student.is_active = true;
    if (student.created_at.empty()) {
        student.created_at = utils::getCurrentTimestamp();
    }

    students.push_back(student);
    if (!storage_->saveStudents(students)) {
        return ServiceResult<Student>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<Student>::ok(student, 201);
}

ServiceResult<std::vector<Student>> StudentManager::getAllStudents(bool include_inactive) {
    auto students = storage_->loadStudents();
    if (include_inactive) {
        return ServiceResult<std::vector<Student>>::ok(students, 200);
    }

    std::vector<Student> active_list;
    for (const auto& s : students) {
        if (s.is_active) {
            active_list.push_back(s);
        }
    }
    return ServiceResult<std::vector<Student>>::ok(active_list, 200);
}

ServiceResult<Student> StudentManager::getStudentById(const std::string& id) {
    std::string clean_id = utils::trim(id);
    if (clean_id.empty()) {
        return ServiceResult<Student>::fail("Student ID cannot be empty.", 400);
    }

    auto students = storage_->loadStudents();
    for (const auto& s : students) {
        if (s.id == clean_id) {
            return ServiceResult<Student>::ok(s, 200);
        }
    }
    return ServiceResult<Student>::fail("Student with ID '" + clean_id + "' not found.", 404);
}

ServiceResult<Student> StudentManager::getStudentByRollNumber(const std::string& roll_number) {
    std::string roll_upper = utils::toUpper(utils::trim(roll_number));
    if (roll_upper.empty()) {
        return ServiceResult<Student>::fail("Roll number cannot be empty.", 400);
    }

    auto students = storage_->loadStudents();
    for (const auto& s : students) {
        if (utils::toUpper(s.roll_number) == roll_upper) {
            return ServiceResult<Student>::ok(s, 200);
        }
    }
    return ServiceResult<Student>::fail("Student with roll number '" + roll_number + "' not found.", 404);
}

ServiceResult<std::vector<Student>> StudentManager::searchStudents(const std::string& query) {
    std::string q = utils::toUpper(utils::trim(query));
    auto students = storage_->loadStudents();

    if (q.empty()) {
        return ServiceResult<std::vector<Student>>::ok(students, 200);
    }

    std::vector<Student> matched;
    for (const auto& s : students) {
        std::string roll_up = utils::toUpper(s.roll_number);
        std::string name_up = utils::toUpper(s.full_name);
        std::string email_up = utils::toUpper(s.email);
        std::string dept_up = utils::toUpper(s.department);
        std::string div_up = utils::toUpper(s.division);

        if (roll_up.find(q) != std::string::npos ||
            name_up.find(q) != std::string::npos ||
            email_up.find(q) != std::string::npos ||
            dept_up.find(q) != std::string::npos ||
            div_up.find(q) != std::string::npos) {
            matched.push_back(s);
        }
    }
    return ServiceResult<std::vector<Student>>::ok(matched, 200);
}

ServiceResult<Student> StudentManager::updateStudent(const std::string& id, const Student& updated_data) {
    std::string clean_id = utils::trim(id);
    if (clean_id.empty()) {
        return ServiceResult<Student>::fail("Student ID cannot be empty.", 400);
    }

    auto val_res = validateStudentFields(updated_data, true);
    if (!val_res.success) {
        return ServiceResult<Student>::fail(val_res.error_message, val_res.http_status);
    }

    auto students = storage_->loadStudents();
    auto it = std::find_if(students.begin(), students.end(), [&](const Student& s) {
        return s.id == clean_id;
    });

    if (it == students.end()) {
        return ServiceResult<Student>::fail("Student with ID '" + clean_id + "' not found.", 404);
    }

    // Check duplicate roll number conflict if roll number changed
    std::string new_roll_upper = utils::toUpper(utils::trim(updated_data.roll_number));
    for (const auto& s : students) {
        if (s.id != clean_id && utils::toUpper(s.roll_number) == new_roll_upper) {
            return ServiceResult<Student>::fail(
                "Roll number '" + updated_data.roll_number + "' is already assigned to another student.", 409);
        }
    }

    // Update mutable fields
    it->roll_number = utils::trim(updated_data.roll_number);
    it->full_name = utils::trim(updated_data.full_name);
    it->email = utils::trim(updated_data.email);
    it->division = utils::trim(updated_data.division);
    it->department = utils::trim(updated_data.department);
    it->semester = updated_data.semester;
    it->is_active = updated_data.is_active;

    if (!storage_->saveStudents(students)) {
        return ServiceResult<Student>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<Student>::ok(*it, 200);
}

ServiceResult<void> StudentManager::deleteStudent(const std::string& id) {
    std::string clean_id = utils::trim(id);
    if (clean_id.empty()) {
        return ServiceResult<void>::fail("Student ID cannot be empty.", 400);
    }

    auto students = storage_->loadStudents();
    auto it = std::find_if(students.begin(), students.end(), [&](const Student& s) {
        return s.id == clean_id;
    });

    if (it == students.end()) {
        return ServiceResult<void>::fail("Student with ID '" + clean_id + "' not found.", 404);
    }

    // Soft deletion architectural pattern: set is_active to false
    it->is_active = false;

    if (!storage_->saveStudents(students)) {
        return ServiceResult<void>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<void>::ok(200);
}

} // namespace smartclass
