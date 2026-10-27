#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../models/TimetableEntry.hpp"
#include "../storage/FileStorageManager.hpp"
#include "../utils/ServiceResult.hpp"

namespace smartclass {

class TimetableManager {
public:
    explicit TimetableManager(std::shared_ptr<FileStorageManager> storage);

    // Business Operations
    ServiceResult<TimetableEntry> createEntry(TimetableEntry entry);
    ServiceResult<std::vector<TimetableEntry>> getAllEntries();
    ServiceResult<std::vector<TimetableEntry>> getEntriesByWeekday(const std::string& weekday);
    ServiceResult<TimetableEntry> getEntryById(const std::string& id);
    ServiceResult<TimetableEntry> updateEntry(const std::string& id, const TimetableEntry& updated_entry);
    ServiceResult<void> deleteEntry(const std::string& id);

private:
    std::shared_ptr<FileStorageManager> storage_;

    std::string generateEntryId(const std::vector<TimetableEntry>& existing);
    ServiceResult<void> validateEntryFields(const TimetableEntry& entry);
    bool checkCollision(const TimetableEntry& candidate, const std::string& ignore_id, std::string& out_conflict_desc);
};

} // namespace smartclass
