#pragma once

#include <string_view>

namespace anvil::core {

enum class LogLevel { trace, debug, info, warning, error, critical };

[[nodiscard]] constexpr std::string_view to_string(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::trace:
        return "trace";
    case LogLevel::debug:
        return "debug";
    case LogLevel::info:
        return "info";
    case LogLevel::warning:
        return "warning";
    case LogLevel::error:
        return "error";
    case LogLevel::critical:
        return "critical";
    }
    return "unknown";
}

void log(LogLevel level, std::string_view category, std::string_view message);

} // namespace anvil::core
