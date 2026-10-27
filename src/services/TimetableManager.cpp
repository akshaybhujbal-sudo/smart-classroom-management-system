#include "../../include/services/TimetableManager.hpp"
#include "../../include/utils/Validation.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace smartclass {

TimetableManager::TimetableManager(std::shared_ptr<FileStorageManager> storage)
    : storage_(std::move(storage)) {}

std::string TimetableManager::generateEntryId(const std::vector<TimetableEntry>& existing) {
    int max_id = 100;
    for (const auto& e : existing) {
        if (e.id.rfind("tt-", 0) == 0) {
            try {
                int val = std::stoi(e.id.substr(3));
                if (val > max_id) max_id = val;
            } catch (...) {}
        }
    }
    std::ostringstream ss;
    ss << "tt-" << (max_id + 1);
    return ss.str();
}

ServiceResult<void> TimetableManager::validateEntryFields(const TimetableEntry& entry) {
    std::string sub = utils::trim(entry.subject);
    std::string teach = utils::trim(entry.teacher);
    std::string room = utils::trim(entry.classroom);
    std::string day = utils::trim(entry.weekday);
    std::string start = utils::trim(entry.start_time);
    std::string end = utils::trim(entry.end_time);

    if (sub.empty()) {
        return ServiceResult<void>::fail("Subject name or code is required.", 400);
    }
    if (teach.empty()) {
        return ServiceResult<void>::fail("Faculty / Teacher name is required.", 400);
    }
    if (room.empty()) {
        return ServiceResult<void>::fail("Classroom / Hall identifier is required.", 400);
    }
    if (!utils::isValidWeekday(day)) {
        return ServiceResult<void>::fail("Invalid weekday. Allowed values: MONDAY to SUNDAY.", 400);
    }
    if (!utils::isValidTime(start)) {
        return ServiceResult<void>::fail("Invalid start time format. Expected 24-hr HH:MM (00:00 - 23:59).", 400);
    }
    if (!utils::isValidTime(end)) {
        return ServiceResult<void>::fail("Invalid end time format. Expected 24-hr HH:MM (00:00 - 23:59).", 400);
    }
    if (!utils::isEndTimeAfterStartTime(start, end)) {
        return ServiceResult<void>::fail("End time (" + end + ") must be strictly after start time (" + start + ").", 400);
    }

    return ServiceResult<void>::ok();
}

bool TimetableManager::checkCollision(const TimetableEntry& candidate, const std::string& ignore_id, std::string& out_conflict_desc) {
    auto entries = storage_->loadTimetable();
    std::string cand_day = utils::toUpper(utils::trim(candidate.weekday));
    std::string cand_room = utils::toUpper(utils::trim(candidate.classroom));
    std::string cand_teacher = utils::toUpper(utils::trim(candidate.teacher));

    for (const auto& e : entries) {
        if (!ignore_id.empty() && e.id == ignore_id) continue;

        if (utils::toUpper(e.weekday) == cand_day) {
            // Check classroom overlap
            if (utils::toUpper(e.classroom) == cand_room) {
                if (utils::doTimeIntervalsOverlap(candidate.start_time, candidate.end_time, e.start_time, e.end_time)) {
                    out_conflict_desc = "Collision detected: Classroom '" + e.classroom + "' is already booked on " +
                                        e.weekday + " from " + e.start_time + " to " + e.end_time + " for '" + e.subject + "'.";
                    return true;
                }
            }
            // Check faculty overlap
            if (utils::toUpper(e.teacher) == cand_teacher) {
                if (utils::doTimeIntervalsOverlap(candidate.start_time, candidate.end_time, e.start_time, e.end_time)) {
                    out_conflict_desc = "Collision detected: Faculty '" + e.teacher + "' is already scheduled on " +
                                        e.weekday + " from " + e.start_time + " to " + e.end_time + " for '" + e.subject + "'.";
                    return true;
                }
            }
        }
    }
    return false;
}

ServiceResult<TimetableEntry> TimetableManager::createEntry(TimetableEntry entry) {
    entry.subject = utils::trim(entry.subject);
    entry.teacher = utils::trim(entry.teacher);
    entry.classroom = utils::trim(entry.classroom);
    entry.weekday = utils::toUpper(utils::trim(entry.weekday));
    entry.start_time = utils::trim(entry.start_time);
    entry.end_time = utils::trim(entry.end_time);

    auto val_res = validateEntryFields(entry);
    if (!val_res.success) {
        return ServiceResult<TimetableEntry>::fail(val_res.error_message, val_res.http_status);
    }

    std::string conflict_desc;
    if (checkCollision(entry, "", conflict_desc)) {
        return ServiceResult<TimetableEntry>::fail(conflict_desc, 409);
    }

    auto entries = storage_->loadTimetable();
    if (entry.id.empty()) {
        entry.id = generateEntryId(entries);
    }

    entries.push_back(entry);
    if (!storage_->saveTimetable(entries)) {
        return ServiceResult<TimetableEntry>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<TimetableEntry>::ok(entry, 201);
}

ServiceResult<std::vector<TimetableEntry>> TimetableManager::getAllEntries() {
    auto entries = storage_->loadTimetable();
    // Sort chronologically by start_time
    std::sort(entries.begin(), entries.end(), [](const TimetableEntry& a, const TimetableEntry& b) {
        return a.start_time < b.start_time;
    });
    return ServiceResult<std::vector<TimetableEntry>>::ok(entries, 200);
}

ServiceResult<std::vector<TimetableEntry>> TimetableManager::getEntriesByWeekday(const std::string& weekday) {
    std::string day_upper = utils::toUpper(utils::trim(weekday));
    if (!utils::isValidWeekday(day_upper)) {
        return ServiceResult<std::vector<TimetableEntry>>::fail("Invalid weekday specified.", 400);
    }

    auto entries = storage_->loadTimetable();
    std::vector<TimetableEntry> filtered;
    for (const auto& e : entries) {
        if (utils::toUpper(e.weekday) == day_upper) {
            filtered.push_back(e);
        }
    }

    std::sort(filtered.begin(), filtered.end(), [](const TimetableEntry& a, const TimetableEntry& b) {
        return a.start_time < b.start_time;
    });

    return ServiceResult<std::vector<TimetableEntry>>::ok(filtered, 200);
}

ServiceResult<TimetableEntry> TimetableManager::getEntryById(const std::string& id) {
    std::string clean_id = utils::trim(id);
    if (clean_id.empty()) {
        return ServiceResult<TimetableEntry>::fail("Timetable entry ID cannot be empty.", 400);
    }

    auto entries = storage_->loadTimetable();
    for (const auto& e : entries) {
        if (e.id == clean_id) {
            return ServiceResult<TimetableEntry>::ok(e, 200);
        }
    }
    return ServiceResult<TimetableEntry>::fail("Timetable entry with ID '" + clean_id + "' not found.", 404);
}

ServiceResult<TimetableEntry> TimetableManager::updateEntry(const std::string& id, const TimetableEntry& updated_entry) {
    std::string clean_id = utils::trim(id);
    if (clean_id.empty()) {
        return ServiceResult<TimetableEntry>::fail("Timetable entry ID cannot be empty.", 400);
    }

    auto val_res = validateEntryFields(updated_entry);
    if (!val_res.success) {
        return ServiceResult<TimetableEntry>::fail(val_res.error_message, val_res.http_status);
    }

    auto entries = storage_->loadTimetable();
    auto it = std::find_if(entries.begin(), entries.end(), [&](const TimetableEntry& e) {
        return e.id == clean_id;
    });

    if (it == entries.end()) {
        return ServiceResult<TimetableEntry>::fail("Timetable entry with ID '" + clean_id + "' not found.", 404);
    }

    std::string conflict_desc;
    if (checkCollision(updated_entry, clean_id, conflict_desc)) {
        return ServiceResult<TimetableEntry>::fail(conflict_desc, 409);
    }

    it->subject = utils::trim(updated_entry.subject);
    it->teacher = utils::trim(updated_entry.teacher);
    it->classroom = utils::trim(updated_entry.classroom);
    it->weekday = utils::toUpper(utils::trim(updated_entry.weekday));
    it->start_time = utils::trim(updated_entry.start_time);
    it->end_time = utils::trim(updated_entry.end_time);

    if (!storage_->saveTimetable(entries)) {
        return ServiceResult<TimetableEntry>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<TimetableEntry>::ok(*it, 200);
}

ServiceResult<void> TimetableManager::deleteEntry(const std::string& id) {
    std::string clean_id = utils::trim(id);
    if (clean_id.empty()) {
        return ServiceResult<void>::fail("Timetable entry ID cannot be empty.", 400);
    }

    auto entries = storage_->loadTimetable();
    auto it = std::find_if(entries.begin(), entries.end(), [&](const TimetableEntry& e) {
        return e.id == clean_id;
    });

    if (it == entries.end()) {
        return ServiceResult<void>::fail("Timetable entry with ID '" + clean_id + "' not found.", 404);
    }

    entries.erase(it);
    if (!storage_->saveTimetable(entries)) {
        return ServiceResult<void>::fail("Internal storage write failure.", 500);
    }

    return ServiceResult<void>::ok(200);
}

} // namespace smartclass
