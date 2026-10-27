#pragma once

#include <string>
#include <optional>
#include "../third_party/nlohmann/json.hpp"

namespace smartclass {

template <typename T>
struct ServiceResult {
    bool success{false};
    std::string error_message;
    int http_status{200};
    std::optional<T> data;

    static ServiceResult<T> ok(T data_val, int status = 200) {
        ServiceResult<T> r;
        r.success = true;
        r.http_status = status;
        r.data = std::move(data_val);
        return r;
    }

    static ServiceResult<T> fail(const std::string& err, int status = 400) {
        ServiceResult<T> r;
        r.success = false;
        r.error_message = err;
        r.http_status = status;
        return r;
    }
};

// Specialized for void operations
template <>
struct ServiceResult<void> {
    bool success{false};
    std::string error_message;
    int http_status{200};

    static ServiceResult<void> ok(int status = 200) {
        ServiceResult<void> r;
        r.success = true;
        r.http_status = status;
        return r;
    }

    static ServiceResult<void> fail(const std::string& err, int status = 400) {
        ServiceResult<void> r;
        r.success = false;
        r.error_message = err;
        r.http_status = status;
        return r;
    }
};

} // namespace smartclass
