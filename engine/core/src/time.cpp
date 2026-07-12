#include <anvil/core/time.hpp>

#include <algorithm>
#include <stdexcept>

namespace anvil::core {

FixedStepScheduler::FixedStepScheduler(FixedStepConfig config) : config_{config} {
    if (config_.fixed_step <= EngineDuration::zero()) {
        throw std::invalid_argument{"Fixed timestep must be positive"};
    }
    if (config_.maximum_frame_time <= EngineDuration::zero()) {
        throw std::invalid_argument{"Maximum frame time must be positive"};
    }
    if (config_.maximum_updates_per_frame == 0) {
        throw std::invalid_argument{"Maximum updates per frame must be positive"};
    }
}

FrameSchedule FixedStepScheduler::schedule(const EngineDuration elapsed_time) {
    const auto observed_time = std::max(elapsed_time, EngineDuration::zero());
    const auto simulation_time = std::min(observed_time, config_.maximum_frame_time);
    auto dropped_time = observed_time - simulation_time;

    accumulator_ += simulation_time;

    const auto available_updates = accumulator_ / config_.fixed_step;
    const auto update_count = static_cast<std::uint32_t>(std::min<std::int64_t>(
        available_updates, static_cast<std::int64_t>(config_.maximum_updates_per_frame)));
    accumulator_ -= config_.fixed_step * update_count;

    if (accumulator_ >= config_.fixed_step) {
        const auto backlog = (accumulator_ / config_.fixed_step) * config_.fixed_step;
        accumulator_ -= backlog;
        dropped_time += backlog;
    }

    return {
        .observed_frame_time = observed_time,
        .simulation_frame_time = simulation_time,
        .fixed_step = config_.fixed_step,
        .dropped_time = dropped_time,
        .fixed_update_count = update_count,
        .interpolation_alpha = static_cast<double>(accumulator_.count()) /
                               static_cast<double>(config_.fixed_step.count()),
        .was_clamped = dropped_time > EngineDuration::zero(),
    };
}

void FixedStepScheduler::reset() noexcept {
    accumulator_ = EngineDuration::zero();
}

const FixedStepConfig& FixedStepScheduler::config() const noexcept {
    return config_;
}

EngineDuration FixedStepScheduler::accumulator() const noexcept {
    return accumulator_;
}

} // namespace anvil::core
