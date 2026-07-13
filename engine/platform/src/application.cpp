#include <anvil/platform/application.hpp>

#include <anvil/core/log.hpp>

#include <SDL3/SDL.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

void require_sdl(const bool succeeded) {
    if (!succeeded) {
        throw std::runtime_error{SDL_GetError()};
    }
}

class SdlDebugCanvas final : public anvil::platform::DebugCanvas {
  public:
    explicit SdlDebugCanvas(SDL_Window* window) : renderer_{SDL_CreateRenderer(window, nullptr)} {
        if (renderer_ == nullptr) {
            throw std::runtime_error{SDL_GetError()};
        }
    }

    ~SdlDebugCanvas() override { SDL_DestroyRenderer(renderer_); }

    [[nodiscard]] int width() const noexcept override {
        int width{};
        int height{};
        static_cast<void>(SDL_GetRenderOutputSize(renderer_, &width, &height));
        return width;
    }

    [[nodiscard]] int height() const noexcept override {
        int width{};
        int height{};
        static_cast<void>(SDL_GetRenderOutputSize(renderer_, &width, &height));
        return height;
    }

    void begin_frame(const anvil::platform::DebugColor clear_color) override {
        set_color(clear_color);
        require_sdl(SDL_RenderClear(renderer_));
    }

    void draw_line(const float x1, const float y1, const float x2, const float y2,
                   const anvil::platform::DebugColor color) override {
        set_color(color);
        require_sdl(SDL_RenderLine(renderer_, x1, y1, x2, y2));
    }

    void draw_rect(const anvil::platform::DebugRect rectangle,
                   const anvil::platform::DebugColor color) override {
        set_color(color);
        const SDL_FRect sdl_rectangle{rectangle.x, rectangle.y, rectangle.width, rectangle.height};
        require_sdl(SDL_RenderRect(renderer_, &sdl_rectangle));
    }

    void fill_rect(const anvil::platform::DebugRect rectangle,
                   const anvil::platform::DebugColor color) override {
        set_color(color);
        const SDL_FRect sdl_rectangle{rectangle.x, rectangle.y, rectangle.width, rectangle.height};
        require_sdl(SDL_RenderFillRect(renderer_, &sdl_rectangle));
    }

    void draw_text(const float x, const float y, const std::string_view text,
                   const anvil::platform::DebugColor color) override {
        set_color(color);
        const std::string owned_text{text};
        require_sdl(SDL_RenderDebugText(renderer_, x, y, owned_text.c_str()));
    }

    void end_frame() override { require_sdl(SDL_RenderPresent(renderer_)); }

  private:
    void set_color(const anvil::platform::DebugColor color) {
        require_sdl(SDL_SetRenderDrawColor(renderer_, color.red, color.green, color.blue,
                                           color.alpha));
    }

    SDL_Renderer* renderer_{};
};

} // namespace

namespace anvil::platform {

struct Application::State {
    SDL_Window* window{nullptr};
    core::FixedStepConfig timing{};
    std::unique_ptr<DebugCanvas> debug_canvas;
    bool exit_after_first_frame{false};
    bool exit_requested{false};
};

Application::Application(ApplicationConfig config)
    : state_{std::make_unique<State>(nullptr, config.timing, nullptr,
                                    config.exit_after_first_frame, false)} {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        throw std::runtime_error{SDL_GetError()};
    }

    state_->window = SDL_CreateWindow(config.name.c_str(), config.width, config.height,
                                      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (state_->window == nullptr) {
        const std::string error{SDL_GetError()};
        SDL_Quit();
        throw std::runtime_error{error};
    }

    core::log(core::LogLevel::info, "platform", "SDL3 application initialized");
}

Application::~Application() {
    state_->debug_canvas.reset();
    SDL_DestroyWindow(state_->window);
    SDL_Quit();
}

void ApplicationHooks::on_start(Application&) {}

void ApplicationHooks::on_fixed_update(Application&, core::EngineDuration) {}

void ApplicationHooks::on_update(Application&, const core::FrameSchedule&) {}

void ApplicationHooks::on_stop(Application&) noexcept {}

int Application::run(ApplicationHooks& hooks) {
    core::FixedStepScheduler scheduler{state_->timing};
    auto previous_frame = std::chrono::steady_clock::now();

    hooks.on_start(*this);
    try {
        while (!state_->exit_requested) {
            SDL_Event event{};
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    request_exit();
                }
            }
            if (state_->exit_requested) {
                break;
            }

            const auto current_frame = std::chrono::steady_clock::now();
            const auto elapsed_time =
                std::chrono::duration_cast<core::EngineDuration>(current_frame - previous_frame);
            previous_frame = current_frame;

            const auto frame = scheduler.schedule(elapsed_time);
            for (std::uint32_t update = 0; update < frame.fixed_update_count; ++update) {
                hooks.on_fixed_update(*this, frame.fixed_step);
                if (state_->exit_requested) {
                    break;
                }
            }

            if (!state_->exit_requested) {
                hooks.on_update(*this, frame);
            }
            if (state_->exit_after_first_frame) {
                request_exit();
            }
            SDL_Delay(1);
        }
    } catch (...) {
        hooks.on_stop(*this);
        throw;
    }

    hooks.on_stop(*this);
    return 0;
}

void Application::request_exit() noexcept {
    state_->exit_requested = true;
}

void Application::set_title(const std::string_view title) {
    const std::string owned_title{title};
    require_sdl(SDL_SetWindowTitle(state_->window, owned_title.c_str()));
}

bool Application::exit_requested() const noexcept {
    return state_->exit_requested;
}

DebugCanvas& Application::debug_canvas() {
    if (!state_->debug_canvas) {
        state_->debug_canvas = std::make_unique<SdlDebugCanvas>(state_->window);
    }
    return *state_->debug_canvas;
}

} // namespace anvil::platform
