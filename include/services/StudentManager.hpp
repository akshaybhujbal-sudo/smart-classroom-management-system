#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../models/Student.hpp"
#include "../storage/FileStorageManager.hpp"
#include "../utils/ServiceResult.hpp"

namespace smartclass {

class StudentManager {
public:
    explicit StudentManager(std::shared_ptr<FileStorageManager> storage);

    // Business Operations
    ServiceResult<Student> addStudent(Student student);
    ServiceResult<std::vector<Student>> getAllStudents(bool include_inactive = true);
    ServiceResult<Student> getStudentById(const std::string& id);
    ServiceResult<Student> getStudentByRollNumber(const std::string& roll_number);
    ServiceResult<std::vector<Student>> searchStudents(const std::string& query);
    ServiceResult<Student> updateStudent(const std::string& id, const Student& updated_data);
    ServiceResult<void> deleteStudent(const std::string& id); // Soft delete

private:
    std::shared_ptr<FileStorageManager> storage_;

    std::string generateStudentId(const std::vector<Student>& existing);
    ServiceResult<void> validateStudentFields(const Student& s, bool is_update = false);
};

} // namespace smartclass
