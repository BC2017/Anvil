#include <anvil/core/log.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace anvil::core {
namespace {

std::mutex log_mutex;

} // namespace

void log(const LogLevel level, const std::string_view category, const std::string_view message) {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time{};
#ifdef _WIN32
    localtime_s(&local_time, &time);
#else
    localtime_r(&time, &local_time);
#endif

    const std::scoped_lock lock{log_mutex};
    std::clog << std::put_time(&local_time, "%Y-%m-%dT%H:%M:%S") << " [" << to_string(level)
              << "] [" << category << "] " << message << '\n';
}

} // namespace anvil::core
