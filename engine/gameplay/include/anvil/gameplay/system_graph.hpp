#pragma once

#include <anvil/core/time.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace anvil::gameplay {

enum class SystemPhase : std::uint8_t {
    pre_simulation,
    simulation,
    post_simulation,
};

struct FixedUpdateContext {
    core::EngineDuration delta_time{};
    std::uint64_t tick_index{};
};

struct SystemDescriptor {
    std::string name;
    SystemPhase phase{SystemPhase::simulation};
    std::vector<std::string> runs_after{};
};

using FixedUpdateCallback = std::function<void(const FixedUpdateContext&)>;

class SystemGraph final {
  public:
    void add(SystemDescriptor descriptor, FixedUpdateCallback callback);
    void clear() noexcept;

    void build();
    void execute_fixed_step(core::EngineDuration delta_time);

    [[nodiscard]] bool built() const noexcept;
    [[nodiscard]] std::size_t system_count() const noexcept;
    [[nodiscard]] std::uint64_t tick_index() const noexcept;
    [[nodiscard]] std::vector<std::string> execution_order() const;

  private:
    struct SystemEntry {
        SystemDescriptor descriptor;
        FixedUpdateCallback callback;
    };

    std::vector<SystemEntry> systems_;
    std::vector<std::size_t> execution_order_;
    std::uint64_t tick_index_{};
    bool built_{};
};

[[nodiscard]] std::string_view to_string(SystemPhase phase) noexcept;

} // namespace anvil::gameplay
