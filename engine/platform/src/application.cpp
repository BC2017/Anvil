#include <anvil/platform/application.hpp>

#include <anvil/core/log.hpp>

#include <SDL3/SDL.h>

#include <chrono>
#include <stdexcept>

namespace anvil::platform {

struct Application::State {
    SDL_Window* window{nullptr};
    core::FixedStepConfig timing{};
    bool exit_after_first_frame{false};
    bool exit_requested{false};
};

Application::Application(ApplicationConfig config)
    : state_{std::make_unique<State>(nullptr, config.timing, config.exit_after_first_frame, false)} {
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

bool Application::exit_requested() const noexcept {
    return state_->exit_requested;
}

} // namespace anvil::platform
