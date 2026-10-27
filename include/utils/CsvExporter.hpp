#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace smartclass {
namespace utils {

class CsvExporter {
public:
    static std::string escapeCsvField(const std::string& field) {
        bool needs_quotes = false;
        if (field.find(',') != std::string::npos ||
            field.find('"') != std::string::npos ||
            field.find('\n') != std::string::npos ||
            field.find('\r') != std::string::npos) {
            needs_quotes = true;
        }

        std::string escaped = "";
        for (char c : field) {
            if (c == '"') {
                escaped += "\"\""; // Escape quotes by doubling them (RFC 4180)
            } else {
                escaped += c;
            }
        }

        if (needs_quotes) {
            return "\"" + escaped + "\"";
        }
        return escaped;
    }

    static std::string buildRow(const std::vector<std::string>& fields) {
        std::ostringstream ss;
        for (size_t i = 0; i < fields.size(); ++i) {
            ss << escapeCsvField(fields[i]);
            if (i + 1 < fields.size()) {
                ss << ",";
            }
        }
        ss << "\r\n";
        return ss.str();
    }
};

} // namespace utils
} // namespace smartclass
