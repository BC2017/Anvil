#pragma once

#include <chrono>
#include <cstdint>

namespace anvil::core {

using EngineDuration = std::chrono::nanoseconds;

struct FixedStepConfig {
    EngineDuration fixed_step{std::chrono::nanoseconds{16'666'667}};
    EngineDuration maximum_frame_time{std::chrono::milliseconds{250}};
    std::uint32_t maximum_updates_per_frame{8};
};

struct FrameSchedule {
    EngineDuration observed_frame_time{};
    EngineDuration simulation_frame_time{};
    EngineDuration fixed_step{};
    EngineDuration dropped_time{};
    std::uint32_t fixed_update_count{};
    double interpolation_alpha{};
    bool was_clamped{};
};

class FixedStepScheduler final {
  public:
    explicit FixedStepScheduler(FixedStepConfig config = {});

    [[nodiscard]] FrameSchedule schedule(EngineDuration elapsed_time);
    void reset() noexcept;

    [[nodiscard]] const FixedStepConfig& config() const noexcept;
    [[nodiscard]] EngineDuration accumulator() const noexcept;

  private:
    FixedStepConfig config_;
    EngineDuration accumulator_{};
};

} // namespace anvil::core
