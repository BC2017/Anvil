#pragma once

#include <cstdint>
#include <string_view>

namespace anvil::platform {

struct DebugColor {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};
    std::uint8_t alpha{255};
};

struct DebugRect {
    float x{};
    float y{};
    float width{};
    float height{};
};

class DebugCanvas {
  public:
    virtual ~DebugCanvas() = default;

    [[nodiscard]] virtual int width() const noexcept = 0;
    [[nodiscard]] virtual int height() const noexcept = 0;

    virtual void begin_frame(DebugColor clear_color) = 0;
    virtual void draw_line(float x1, float y1, float x2, float y2, DebugColor color) = 0;
    virtual void draw_rect(DebugRect rectangle, DebugColor color) = 0;
    virtual void fill_rect(DebugRect rectangle, DebugColor color) = 0;
    virtual void draw_text(float x, float y, std::string_view text, DebugColor color) = 0;
    virtual void end_frame() = 0;
};

} // namespace anvil::platform
