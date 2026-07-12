#include <anvil/core/log.hpp>
#include <anvil/core/time.hpp>
#include <anvil/core/version.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failures;
    }
}

void test_fixed_step_scheduler() {
    using namespace std::chrono_literals;

    anvil::core::FixedStepScheduler scheduler{{
        .fixed_step = 10ms,
        .maximum_frame_time = 100ms,
        .maximum_updates_per_frame = 8,
    }};

    const auto partial_frame = scheduler.schedule(6ms);
    expect(partial_frame.fixed_update_count == 0, "partial step does not update simulation");
    expect(std::abs(partial_frame.interpolation_alpha - 0.6) < 0.000'001,
           "partial step exposes interpolation alpha");

    const auto completed_step = scheduler.schedule(6ms);
    expect(completed_step.fixed_update_count == 1, "accumulated time produces a fixed update");
    expect(scheduler.accumulator() == 2ms, "unconsumed time remains in the accumulator");

    scheduler.reset();
    expect(scheduler.accumulator() == 0ns, "reset clears accumulated time");
}

void test_scheduler_clamps_stalls_and_backlogs() {
    using namespace std::chrono_literals;

    anvil::core::FixedStepScheduler frame_clamp{{
        .fixed_step = 10ms,
        .maximum_frame_time = 25ms,
        .maximum_updates_per_frame = 8,
    }};
    const auto clamped = frame_clamp.schedule(100ms);
    expect(clamped.fixed_update_count == 2, "long frame is limited by maximum frame time");
    expect(clamped.dropped_time == 75ms, "clamped frame reports dropped wall-clock time");
    expect(clamped.was_clamped, "long frame is marked as clamped");

    anvil::core::FixedStepScheduler backlog_clamp{{
        .fixed_step = 10ms,
        .maximum_frame_time = 100ms,
        .maximum_updates_per_frame = 3,
    }};
    const auto backlog = backlog_clamp.schedule(95ms);
    expect(backlog.fixed_update_count == 3, "update cap prevents a simulation death spiral");
    expect(backlog.dropped_time == 60ms, "discarded backlog is reported");
    expect(backlog_clamp.accumulator() == 5ms, "sub-step backlog remains for interpolation");

    const auto negative = backlog_clamp.schedule(-5ms);
    expect(negative.observed_frame_time == 0ns, "negative elapsed time is sanitized");
}

void test_scheduler_rejects_invalid_configuration() {
    using namespace std::chrono_literals;

    bool threw = false;
    try {
        [[maybe_unused]] anvil::core::FixedStepScheduler invalid{{.fixed_step = 0ns}};
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "zero fixed timestep is rejected");
}
} // namespace

int main() {
    using anvil::core::LogLevel;

    expect(anvil::core::version_major == 0, "development version has major version zero");
    expect(!anvil::core::version_string.empty(), "generated version string is non-empty");
    expect(anvil::core::to_string(LogLevel::trace) == "trace", "trace level has stable text");
    expect(anvil::core::to_string(LogLevel::critical) == "critical",
           "critical level has stable text");
    test_fixed_step_scheduler();
    test_scheduler_clamps_stalls_and_backlogs();
    test_scheduler_rejects_invalid_configuration();

    if (failures == 0) {
        std::cout << "All core tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
