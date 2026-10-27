#pragma once

#include <string>
#include <algorithm>
#include <regex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace smartclass {
namespace utils {

inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

inline std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return str;
}

inline bool isValidEmail(const std::string& email) {
    if (email.empty()) return false;
    // Standard RFC-compatible regex pattern for email
    const std::regex email_pattern(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, email_pattern);
}

inline bool isValidDate(const std::string& date) {
    // Expected format: YYYY-MM-DD
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (size_t i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;

    // Days in months
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    // Leap year check
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    if (day < 1 || day > days_in_month[month - 1]) return false;

    return true;
}

inline bool isValidTime(const std::string& time) {
    // Expected format: HH:MM (24-hour)
    if (time.length() != 5 || time[2] != ':') return false;
    if (!std::isdigit(static_cast<unsigned char>(time[0])) ||
        !std::isdigit(static_cast<unsigned char>(time[1])) ||
        !std::isdigit(static_cast<unsigned char>(time[3])) ||
        !std::isdigit(static_cast<unsigned char>(time[4]))) {
        return false;
    }
    int hours = std::stoi(time.substr(0, 2));
    int minutes = std::stoi(time.substr(3, 2));
    return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
}

inline int timeToMinutes(const std::string& time) {
    if (!isValidTime(time)) return -1;
    int hours = std::stoi(time.substr(0, 2));
    int minutes = std::stoi(time.substr(3, 2));
    return hours * 60 + minutes;
}

inline bool isEndTimeAfterStartTime(const std::string& start, const std::string& end) {
    int s = timeToMinutes(start);
    int e = timeToMinutes(end);
    if (s < 0 || e < 0) return false;
    return e > s;
}

inline bool doTimeIntervalsOverlap(const std::string& start1, const std::string& end1,
                                  const std::string& start2, const std::string& end2) {
    int s1 = timeToMinutes(start1);
    int e1 = timeToMinutes(end1);
    int s2 = timeToMinutes(start2);
    int e2 = timeToMinutes(end2);
    if (s1 < 0 || e1 < 0 || s2 < 0 || e2 < 0) return false;
    return std::max(s1, s2) < std::min(e1, e2);
}

inline bool isValidWeekday(const std::string& day) {
    std::string upper = toUpper(trim(day));
    return (upper == "MONDAY" || upper == "TUESDAY" || upper == "WEDNESDAY" ||
            upper == "THURSDAY" || upper == "FRIDAY" || upper == "SATURDAY" ||
            upper == "SUNDAY");
}

inline std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    gmtime_s(&tm_buf, &now_c);
#else
    gmtime_r(&now_c, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

inline std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &now_c);
#else
    localtime_r(&now_c, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d");
    return oss.str();
}

} // namespace utils
} // namespace smartclass
