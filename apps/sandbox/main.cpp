#include <anvil/core/log.hpp>
#include <anvil/core/version.hpp>
#include <anvil/platform/application.hpp>

#include <exception>
#include <string>
#include <string_view>

int main(const int argument_count, char* arguments[]) {
    using anvil::core::LogLevel;

    try {
        anvil::core::log(LogLevel::info, "sandbox",
                         std::string{"Starting Anvil "} +
                             std::string{anvil::core::version_string});
        const bool smoke_test = argument_count > 1 &&
                                std::string_view{arguments[1]} == "--smoke-test";
        anvil::platform::Application application{
            {.name = "Anvil Sandbox", .exit_after_first_frame = smoke_test}};
        return application.run();
    } catch (const std::exception& exception) {
        anvil::core::log(LogLevel::critical, "sandbox", exception.what());
        return 1;
    }
}
