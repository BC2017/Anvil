#include <anvil/gameplay/system_graph.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failures;
    }
}

template <typename Exception, typename Callback>
void expect_throws(Callback&& callback, const std::string_view description) {
    try {
        callback();
        expect(false, description);
    } catch (const Exception&) {
        expect(true, description);
    }
}

void test_phase_and_dependency_ordering_is_deterministic() {
    anvil::gameplay::SystemGraph graph;
    std::vector<std::string> calls;
    const auto record = [&](const std::string_view name) {
        return [&, name](const anvil::gameplay::FixedUpdateContext&) {
            calls.emplace_back(name);
        };
    };

    graph.add({.name = "animation",
               .phase = anvil::gameplay::SystemPhase::simulation,
               .runs_after = {"movement"}},
              record("animation"));
    graph.add({.name = "render extraction",
               .phase = anvil::gameplay::SystemPhase::post_simulation,
               .runs_after = {"animation"}},
              record("render extraction"));
    graph.add({.name = "input", .phase = anvil::gameplay::SystemPhase::pre_simulation},
              record("input"));
    graph.add({.name = "movement",
               .phase = anvil::gameplay::SystemPhase::simulation,
               .runs_after = {"input"}},
              record("movement"));

    graph.build();
    const std::vector<std::string> expected{"input", "movement", "animation",
                                            "render extraction"};
    expect(graph.execution_order() == expected, "build produces stable phase/dependency order");

    graph.execute_fixed_step(std::chrono::milliseconds{16});
    expect(calls == expected, "fixed update executes the compiled order");
}

void test_context_and_tick_progression() {
    anvil::gameplay::SystemGraph graph;
    std::vector<anvil::gameplay::FixedUpdateContext> contexts;
    graph.add({.name = "capture"}, [&](const anvil::gameplay::FixedUpdateContext& context) {
        contexts.push_back(context);
    });
    graph.build();

    graph.execute_fixed_step(std::chrono::milliseconds{20});
    graph.execute_fixed_step(std::chrono::milliseconds{20});
    expect(contexts.size() == 2, "system executes once per fixed step");
    expect(contexts[0].delta_time == std::chrono::milliseconds{20} &&
               contexts[0].tick_index == 0 && contexts[1].tick_index == 1,
           "fixed update context carries delta and monotonic tick");
    expect(graph.tick_index() == 2, "graph reports completed tick count");
}

void test_graph_validation() {
    anvil::gameplay::SystemGraph graph;
    const auto no_op = [](const anvil::gameplay::FixedUpdateContext&) {};
    graph.add({.name = "first", .runs_after = {"missing"}}, no_op);
    expect_throws<std::invalid_argument>([&] { graph.build(); },
                                         "missing dependency is rejected");
    expect(!graph.built(), "failed build leaves graph unbuilt");

    graph.clear();
    graph.add({.name = "first", .runs_after = {"second"}}, no_op);
    graph.add({.name = "second", .runs_after = {"first"}}, no_op);
    expect_throws<std::logic_error>([&] { graph.build(); }, "dependency cycle is rejected");

    graph.clear();
    graph.add({.name = "late", .phase = anvil::gameplay::SystemPhase::post_simulation}, no_op);
    graph.add({.name = "early",
               .phase = anvil::gameplay::SystemPhase::pre_simulation,
               .runs_after = {"late"}},
              no_op);
    expect_throws<std::invalid_argument>([&] { graph.build(); },
                                         "backwards phase dependency is rejected");

    expect_throws<std::invalid_argument>(
        [&] {
            graph.add({.name = "invalid phase", .phase = static_cast<anvil::gameplay::SystemPhase>(99)},
                      no_op);
        },
        "out-of-range phase is rejected during registration");
}

void test_mutation_and_execution_guards() {
    anvil::gameplay::SystemGraph graph;
    const auto no_op = [](const anvil::gameplay::FixedUpdateContext&) {};
    graph.add({.name = "first"}, no_op);
    graph.build();
    graph.add({.name = "second"}, no_op);
    expect(!graph.built(), "adding a system invalidates compiled order");
    expect_throws<std::logic_error>(
        [&] { graph.execute_fixed_step(std::chrono::milliseconds{16}); },
        "unbuilt graph cannot execute");

    graph.build();
    expect_throws<std::invalid_argument>(
        [&] { graph.execute_fixed_step(anvil::core::EngineDuration::zero()); },
        "non-positive fixed delta is rejected");
    expect(graph.tick_index() == 0, "rejected execution does not advance tick");
}

void test_callback_failure_does_not_complete_tick() {
    anvil::gameplay::SystemGraph graph;
    graph.add({.name = "failure"}, [](const anvil::gameplay::FixedUpdateContext&) {
        throw std::runtime_error{"system failed"};
    });
    graph.build();
    expect_throws<std::runtime_error>(
        [&] { graph.execute_fixed_step(std::chrono::milliseconds{16}); },
        "system exception propagates to runtime owner");
    expect(graph.tick_index() == 0, "failed system execution does not complete tick");
}

} // namespace

int main() {
    test_phase_and_dependency_ordering_is_deterministic();
    test_context_and_tick_progression();
    test_graph_validation();
    test_mutation_and_execution_guards();
    test_callback_failure_does_not_complete_tick();

    if (failures == 0) {
        std::cout << "All gameplay system graph tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
