#pragma once

#include <memory>
#include <string>

namespace anvil::platform {

struct ApplicationConfig {
    std::string name{"Anvil"};
    int width{1280};
    int height{720};
    bool exit_after_first_frame{false};
};

class Application final {
  public:
    explicit Application(ApplicationConfig config);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    [[nodiscard]] int run();

  private:
    struct State;
    std::unique_ptr<State> state_;
};

} // namespace anvil::platform
