#include <anvil/core/assert.hpp>

#include <anvil/core/log.hpp>

#include <cstdlib>
#include <string>

namespace anvil::core {

[[noreturn]] void assertion_failed(const std::string_view expression,
                                   const std::string_view message,
                                   const std::source_location& location) {
    const auto details = std::string{"Assertion `"} + std::string{expression} + "` failed: " +
                         std::string{message} + " at " + location.file_name() + ':' +
                         std::to_string(location.line());
    log(LogLevel::critical, "assert", details);
    std::abort();
}

} // namespace anvil::core
