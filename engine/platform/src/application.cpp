#include <anvil/platform/application.hpp>

#include <anvil/core/log.hpp>

#include <SDL3/SDL.h>

#include <stdexcept>

namespace anvil::platform {

struct Application::State {
    SDL_Window* window{nullptr};
    bool exit_after_first_frame{false};
};

Application::Application(ApplicationConfig config)
    : state_{std::make_unique<State>(nullptr, config.exit_after_first_frame)} {
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

int Application::run() {
    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        if (state_->exit_after_first_frame) {
            running = false;
        }
        SDL_Delay(1);
    }
    return 0;
}

} // namespace anvil::platform
