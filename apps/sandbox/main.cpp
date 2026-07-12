#include <anvil/core/log.hpp>
#include <anvil/core/version.hpp>
#include <anvil/platform/application.hpp>

#include <exception>
#include <string>
#include <string_view>

namespace {

class SandboxHooks final : public anvil::platform::ApplicationHooks {
  public:
    void on_start(anvil::platform::Application&) override {
        ++start_count_;
        anvil::core::log(anvil::core::LogLevel::info, "sandbox", "Application started");
    }

    void on_update(anvil::platform::Application&,
                   const anvil::core::FrameSchedule&) override {
        ++update_count_;
    }

    void on_stop(anvil::platform::Application&) noexcept override {
        ++stop_count_;
        anvil::core::log(anvil::core::LogLevel::info, "sandbox", "Application stopped");
    }

    [[nodiscard]] bool completed_lifecycle() const noexcept {
        return start_count_ == 1 && update_count_ >= 1 && stop_count_ == 1;
    }

  private:
    int start_count_{};
    int update_count_{};
    int stop_count_{};
};

} // namespace

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
        SandboxHooks hooks;
        const int result = application.run(hooks);
        if (smoke_test && !hooks.completed_lifecycle()) {
            anvil::core::log(LogLevel::critical, "sandbox", "Lifecycle smoke test failed");
            return 2;
        }
        return result;
    } catch (const std::exception& exception) {
        anvil::core::log(LogLevel::critical, "sandbox", exception.what());
        return 1;
    }
}
