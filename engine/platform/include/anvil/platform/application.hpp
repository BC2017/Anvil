#pragma once

#include <anvil/core/time.hpp>
#include <anvil/platform/debug_canvas.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace anvil::platform {

struct ApplicationConfig {
    std::string name{"Anvil"};
    int width{1280};
    int height{720};
    core::FixedStepConfig timing{};
    bool exit_after_first_frame{false};
};

class Application;

class ApplicationHooks {
  public:
    virtual ~ApplicationHooks() = default;

    virtual void on_start(Application& application);
    virtual void on_fixed_update(Application& application, core::EngineDuration fixed_step);
    virtual void on_update(Application& application, const core::FrameSchedule& frame);
    virtual void on_stop(Application& application) noexcept;
};

class Application final {
  public:
    explicit Application(ApplicationConfig config);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    [[nodiscard]] int run(ApplicationHooks& hooks);
    void request_exit() noexcept;
    void set_title(std::string_view title);

    [[nodiscard]] bool exit_requested() const noexcept;
    [[nodiscard]] DebugCanvas& debug_canvas();

  private:
    struct State;
    std::unique_ptr<State> state_;
};

} // namespace anvil::platform
