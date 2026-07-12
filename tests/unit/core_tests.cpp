#include <anvil/core/log.hpp>
#include <anvil/core/version.hpp>

#include <iostream>
#include <string_view>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failures;
    }
}
} // namespace

int main() {
    using anvil::core::LogLevel;

    expect(anvil::core::version_major == 0, "development version has major version zero");
    expect(!anvil::core::version_string.empty(), "generated version string is non-empty");
    expect(anvil::core::to_string(LogLevel::trace) == "trace", "trace level has stable text");
    expect(anvil::core::to_string(LogLevel::critical) == "critical",
           "critical level has stable text");

    if (failures == 0) {
        std::cout << "All core tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
