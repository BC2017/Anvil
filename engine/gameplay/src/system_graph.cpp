#include <anvil/gameplay/system_graph.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace anvil::gameplay {
namespace {

[[nodiscard]] constexpr auto phase_index(const SystemPhase phase) noexcept {
    return static_cast<std::uint8_t>(phase);
}

} // namespace

void SystemGraph::add(SystemDescriptor descriptor, FixedUpdateCallback callback) {
    if (descriptor.name.empty()) {
        throw std::invalid_argument{"Gameplay system name must not be empty"};
    }
    if (!callback) {
        throw std::invalid_argument{"Gameplay system callback must not be empty"};
    }
    if (phase_index(descriptor.phase) > phase_index(SystemPhase::post_simulation)) {
        throw std::invalid_argument{"Gameplay system phase is invalid"};
    }
    const auto duplicate = std::ranges::find_if(systems_, [&](const SystemEntry& entry) {
        return entry.descriptor.name == descriptor.name;
    });
    if (duplicate != systems_.end()) {
        throw std::invalid_argument{"Gameplay system names must be unique: " + descriptor.name};
    }

    std::unordered_set<std::string_view> dependencies;
    for (const auto& dependency : descriptor.runs_after) {
        if (dependency.empty()) {
            throw std::invalid_argument{"Gameplay system dependency must not be empty"};
        }
        if (!dependencies.emplace(dependency).second) {
            throw std::invalid_argument{"Duplicate dependency for gameplay system: " +
                                        descriptor.name};
        }
    }

    systems_.push_back({.descriptor = std::move(descriptor), .callback = std::move(callback)});
    execution_order_.clear();
    built_ = false;
}

void SystemGraph::clear() noexcept {
    systems_.clear();
    execution_order_.clear();
    tick_index_ = 0;
    built_ = false;
}

void SystemGraph::build() {
    std::unordered_map<std::string_view, std::size_t> indices;
    indices.reserve(systems_.size());
    for (std::size_t index = 0; index < systems_.size(); ++index) {
        indices.emplace(systems_[index].descriptor.name, index);
    }

    std::vector<std::vector<std::size_t>> dependants(systems_.size());
    std::vector<std::size_t> indegree(systems_.size());
    for (std::size_t index = 0; index < systems_.size(); ++index) {
        const auto& descriptor = systems_[index].descriptor;
        for (const auto& dependency_name : descriptor.runs_after) {
            const auto dependency = indices.find(dependency_name);
            if (dependency == indices.end()) {
                throw std::invalid_argument{"Unknown gameplay system dependency '" +
                                            dependency_name + "' required by '" + descriptor.name +
                                            "'"};
            }

            const auto dependency_index = dependency->second;
            const auto dependency_phase = systems_[dependency_index].descriptor.phase;
            if (phase_index(dependency_phase) > phase_index(descriptor.phase)) {
                throw std::invalid_argument{"Gameplay system '" + descriptor.name +
                                            "' cannot run after later-phase system '" +
                                            dependency_name + "'"};
            }
            if (dependency_phase == descriptor.phase) {
                dependants[dependency_index].push_back(index);
                ++indegree[index];
            }
        }
    }

    std::vector<std::size_t> order;
    order.reserve(systems_.size());
    for (std::uint8_t phase = phase_index(SystemPhase::pre_simulation);
         phase <= phase_index(SystemPhase::post_simulation); ++phase) {
        std::size_t phase_count{};
        while (true) {
            auto candidate = std::numeric_limits<std::size_t>::max();
            for (std::size_t index = 0; index < systems_.size(); ++index) {
                if (phase_index(systems_[index].descriptor.phase) == phase &&
                    indegree[index] == 0 &&
                    std::ranges::find(order, index) == order.end()) {
                    candidate = index;
                    break;
                }
            }
            if (candidate == std::numeric_limits<std::size_t>::max()) {
                break;
            }

            order.push_back(candidate);
            ++phase_count;
            for (const auto dependant : dependants[candidate]) {
                --indegree[dependant];
            }
        }

        const auto expected = static_cast<std::size_t>(std::ranges::count_if(
            systems_, [phase](const SystemEntry& entry) {
                return phase_index(entry.descriptor.phase) == phase;
            }));
        if (phase_count != expected) {
            throw std::logic_error{"Gameplay system dependency cycle in " +
                                   std::string{to_string(static_cast<SystemPhase>(phase))} +
                                   " phase"};
        }
    }

    execution_order_ = std::move(order);
    built_ = true;
}

void SystemGraph::execute_fixed_step(const core::EngineDuration delta_time) {
    if (!built_) {
        throw std::logic_error{"Gameplay system graph must be built before execution"};
    }
    if (delta_time <= core::EngineDuration::zero()) {
        throw std::invalid_argument{"Gameplay fixed timestep must be positive"};
    }

    const FixedUpdateContext context{.delta_time = delta_time, .tick_index = tick_index_};
    for (const auto index : execution_order_) {
        systems_[index].callback(context);
    }
    ++tick_index_;
}

bool SystemGraph::built() const noexcept {
    return built_;
}

std::size_t SystemGraph::system_count() const noexcept {
    return systems_.size();
}

std::uint64_t SystemGraph::tick_index() const noexcept {
    return tick_index_;
}

std::vector<std::string> SystemGraph::execution_order() const {
    std::vector<std::string> names;
    names.reserve(execution_order_.size());
    for (const auto index : execution_order_) {
        names.push_back(systems_[index].descriptor.name);
    }
    return names;
}

std::string_view to_string(const SystemPhase phase) noexcept {
    switch (phase) {
    case SystemPhase::pre_simulation:
        return "pre-simulation";
    case SystemPhase::simulation:
        return "simulation";
    case SystemPhase::post_simulation:
        return "post-simulation";
    }
    return "unknown";
}

} // namespace anvil::gameplay
