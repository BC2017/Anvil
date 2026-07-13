#include <anvil/core/log.hpp>
#include <anvil/core/version.hpp>
#include <anvil/gameplay/system_graph.hpp>
#include <anvil/platform/application.hpp>
#include <anvil/platform/debug_canvas.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <exception>
#include <string>
#include <string_view>

namespace {

using anvil::platform::DebugCanvas;
using anvil::platform::DebugColor;
using anvil::platform::DebugRect;

constexpr DebugColor background{11, 15, 24, 255};
constexpr DebugColor panel{18, 24, 36, 255};
constexpr DebugColor panel_bright{24, 33, 48, 255};
constexpr DebugColor grid{31, 43, 61, 255};
constexpr DebugColor text{204, 219, 232, 255};
constexpr DebugColor muted{99, 122, 143, 255};
constexpr DebugColor cyan{42, 211, 196, 255};
constexpr DebugColor orange{255, 164, 76, 255};

struct ScreenPoint {
    float x{};
    float y{};
};

void draw_grid(DebugCanvas& canvas, const DebugRect viewport, const float offset) {
    constexpr float spacing = 40.0F;
    const float shifted = std::fmod(offset, spacing);
    for (float x = viewport.x + shifted; x < viewport.x + viewport.width; x += spacing) {
        canvas.draw_line(x, viewport.y, x, viewport.y + viewport.height, grid);
    }
    for (float y = viewport.y + shifted; y < viewport.y + viewport.height; y += spacing) {
        canvas.draw_line(viewport.x, y, viewport.x + viewport.width, y, grid);
    }
}

void draw_cube(DebugCanvas& canvas, const DebugRect viewport, const float angle) {
    struct Point3 {
        float x;
        float y;
        float z;
    };
    constexpr std::array<Point3, 8> vertices{{
        {-1.0F, -1.0F, -1.0F}, {1.0F, -1.0F, -1.0F}, {1.0F, 1.0F, -1.0F},
        {-1.0F, 1.0F, -1.0F},  {-1.0F, -1.0F, 1.0F}, {1.0F, -1.0F, 1.0F},
        {1.0F, 1.0F, 1.0F},    {-1.0F, 1.0F, 1.0F},
    }};
    constexpr std::array<std::array<std::size_t, 2>, 12> edges{{
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
        {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7},
    }};

    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    constexpr float pitch = 0.42F;
    const float pitch_cosine = std::cos(pitch);
    const float pitch_sine = std::sin(pitch);
    const float center_x = viewport.x + viewport.width * 0.54F;
    const float center_y = viewport.y + viewport.height * 0.50F;
    const float base_scale = std::min(viewport.width, viewport.height) * 0.28F;
    std::array<ScreenPoint, vertices.size()> projected{};

    for (std::size_t index = 0; index < vertices.size(); ++index) {
        const auto vertex = vertices[index];
        const float rotated_x = vertex.x * cosine + vertex.z * sine;
        const float rotated_z = -vertex.x * sine + vertex.z * cosine;
        const float rotated_y = vertex.y * pitch_cosine - rotated_z * pitch_sine;
        const float depth = vertex.y * pitch_sine + rotated_z * pitch_cosine;
        const float perspective = base_scale / (3.8F + depth);
        projected[index] = {
            .x = center_x + rotated_x * perspective,
            .y = center_y + rotated_y * perspective,
        };
    }

    for (const auto edge : edges) {
        const auto from = projected[edge[0]];
        const auto to = projected[edge[1]];
        canvas.draw_line(from.x, from.y, to.x, to.y, cyan);
    }

    for (const auto point : projected) {
        canvas.fill_rect({point.x - 3.0F, point.y - 3.0F, 6.0F, 6.0F}, orange);
    }
}

class SandboxHooks final : public anvil::platform::ApplicationHooks {
  public:
    explicit SandboxHooks(const bool visual) : visual_{visual} {}

    void on_start(anvil::platform::Application& application) override {
        ++start_count_;
        application.set_title("Anvil // Gameplay Systems Visualizer");
        configure_systems();
        anvil::core::log(anvil::core::LogLevel::info, "sandbox", "Application started");
    }

    void on_fixed_update(anvil::platform::Application&,
                         const anvil::core::EngineDuration fixed_step) override {
        graph_.execute_fixed_step(fixed_step);
    }

    void on_update(anvil::platform::Application& application,
                   const anvil::core::FrameSchedule& frame) override {
        ++update_count_;
        if (visual_) {
            draw(application.debug_canvas(), frame);
        }
    }

    void on_stop(anvil::platform::Application&) noexcept override {
        ++stop_count_;
        anvil::core::log(anvil::core::LogLevel::info, "sandbox", "Application stopped");
    }

    [[nodiscard]] bool completed_lifecycle() const noexcept {
        return start_count_ == 1 && update_count_ >= 1 && stop_count_ == 1;
    }

  private:
    void configure_systems() {
        graph_.add({.name = "input", .phase = anvil::gameplay::SystemPhase::pre_simulation},
                   [](const anvil::gameplay::FixedUpdateContext&) {});
        graph_.add({.name = "movement",
                    .phase = anvil::gameplay::SystemPhase::simulation,
                    .runs_after = {"input"}},
                   [this](const anvil::gameplay::FixedUpdateContext& context) {
                       const auto seconds =
                           std::chrono::duration<float>(context.delta_time).count();
                       simulation_angle_ += seconds * 0.8F;
                   });
        graph_.add({.name = "animation",
                    .phase = anvil::gameplay::SystemPhase::simulation,
                    .runs_after = {"movement"}},
                   [this](const anvil::gameplay::FixedUpdateContext&) {
                       activity_ = 0.5F + std::sin(simulation_angle_ * 2.0F) * 0.5F;
                   });
        graph_.add({.name = "render extraction",
                    .phase = anvil::gameplay::SystemPhase::post_simulation,
                    .runs_after = {"animation"}},
                   [this](const anvil::gameplay::FixedUpdateContext&) {
                       extracted_angle_ = simulation_angle_;
                   });
        graph_.build();
    }

    void draw(DebugCanvas& canvas, const anvil::core::FrameSchedule& frame) const {
        const float width = static_cast<float>(canvas.width());
        const float height = static_cast<float>(canvas.height());
        const DebugRect viewport{190.0F, 76.0F, std::max(100.0F, width - 214.0F),
                                 std::max(100.0F, height - 124.0F)};
        canvas.begin_frame(background);

        canvas.fill_rect({0.0F, 0.0F, width, 56.0F}, panel);
        canvas.fill_rect({0.0F, 56.0F, 166.0F, height - 56.0F}, panel);
        canvas.fill_rect({166.0F, height - 34.0F, width - 166.0F, 34.0F}, panel);
        canvas.fill_rect(viewport, panel_bright);
        canvas.draw_rect(viewport, grid);
        draw_grid(canvas, viewport, extracted_angle_ * 18.0F);

        const float interpolated_angle =
            extracted_angle_ + static_cast<float>(frame.interpolation_alpha) * 0.013F;
        draw_cube(canvas, viewport, interpolated_angle);

        canvas.draw_text(20.0F, 20.0F, "ANVIL", text);
        canvas.draw_text(76.0F, 20.0F, "// GAMEPLAY SYSTEMS", muted);
        canvas.fill_rect({20.0F, 44.0F, 92.0F, 2.0F}, cyan);

        canvas.draw_text(20.0F, 82.0F, "SYSTEM GRAPH", muted);
        draw_system_row(canvas, 108.0F, "01  INPUT", cyan);
        draw_system_row(canvas, 140.0F, "02  MOVEMENT", text);
        draw_system_row(canvas, 172.0F, "03  ANIMATION", text);
        draw_system_row(canvas, 204.0F, "04  EXTRACTION", orange);

        canvas.draw_text(20.0F, 260.0F, "ACTIVITY", muted);
        canvas.draw_rect({20.0F, 280.0F, 124.0F, 12.0F}, grid);
        canvas.fill_rect({22.0F, 282.0F, 120.0F * activity_, 8.0F}, cyan);

        canvas.draw_text(viewport.x + 18.0F, viewport.y + 18.0F, "LIVE SCENE", text);
        canvas.draw_text(viewport.x + 18.0F, viewport.y + 36.0F,
                         "FIXED STEP / DETERMINISTIC ORDER", muted);
        canvas.draw_text(184.0F, height - 22.0F,
                         "TICK " + std::to_string(graph_.tick_index()), text);
        canvas.draw_text(width - 276.0F, height - 22.0F,
                         frame.was_clamped ? "FRAME CLAMPED" : "SIMULATION NOMINAL",
                         frame.was_clamped ? orange : cyan);
        canvas.end_frame();
    }

    static void draw_system_row(DebugCanvas& canvas, const float y, const std::string_view label,
                                const DebugColor color) {
        canvas.fill_rect({20.0F, y + 2.0F, 3.0F, 18.0F}, color);
        canvas.draw_text(32.0F, y + 7.0F, label, color);
    }

    anvil::gameplay::SystemGraph graph_;
    float simulation_angle_{};
    float extracted_angle_{};
    float activity_{};
    bool visual_{};
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
        const std::string_view mode = argument_count > 1 ? arguments[1] : "";
        const bool smoke_test = mode == "--smoke-test";
        const bool visual_smoke_test = mode == "--visual-smoke-test";
        anvil::platform::Application application{
            {.name = "Anvil Sandbox", .exit_after_first_frame = smoke_test || visual_smoke_test}};
        SandboxHooks hooks{!smoke_test};
        const int result = application.run(hooks);
        if ((smoke_test || visual_smoke_test) && !hooks.completed_lifecycle()) {
            anvil::core::log(LogLevel::critical, "sandbox", "Lifecycle smoke test failed");
            return 2;
        }
        return result;
    } catch (const std::exception& exception) {
        anvil::core::log(LogLevel::critical, "sandbox", exception.what());
        return 1;
    }
}
