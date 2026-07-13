#include <anvil/editor/command_history.hpp>

#include <iostream>
#include <memory>
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

class SetIntegerCommand final : public anvil::editor::EditorCommand {
  public:
    SetIntegerCommand(int& target, const int value, const std::string_view label = "Set integer")
        : target_{target}, previous_{target}, value_{value}, label_{label} {}

    [[nodiscard]] std::string_view name() const noexcept override { return label_; }
    void apply() override { target_ = value_; }
    void undo() override { target_ = previous_; }

  private:
    int& target_;
    int previous_{};
    int value_{};
    std::string_view label_;
};

class ThrowingCommand final : public anvil::editor::EditorCommand {
  public:
    ThrowingCommand(int& target, const bool throw_on_apply, const bool throw_on_undo)
        : target_{target}, throw_on_apply_{throw_on_apply}, throw_on_undo_{throw_on_undo} {}

    [[nodiscard]] std::string_view name() const noexcept override { return "Throwing command"; }

    void apply() override {
        if (throw_on_apply_) {
            throw std::runtime_error{"apply failed"};
        }
        target_ = 1;
    }

    void undo() override {
        if (throw_on_undo_) {
            throw std::runtime_error{"undo failed"};
        }
        target_ = 0;
    }

  private:
    int& target_;
    bool throw_on_apply_{};
    bool throw_on_undo_{};
};

void test_execute_undo_redo_and_saved_state() {
    anvil::editor::CommandHistory history;
    int value{};

    expect(!history.modified(), "new command history begins clean");
    history.execute(std::make_unique<SetIntegerCommand>(value, 10, "Set value to ten"));
    expect(value == 10, "executing command mutates target");
    expect(history.modified(), "executing command marks document modified");
    expect(history.undo_name() == "Set value to ten", "undo label describes pending command");

    expect(history.undo(), "executed command can be undone");
    expect(value == 0, "undo restores previous value");
    expect(!history.modified(), "undoing to initial state restores clean status");
    expect(history.redo_name() == "Set value to ten", "redo label describes pending command");

    expect(history.redo(), "undone command can be redone");
    expect(value == 10, "redo reapplies value");
    history.mark_saved();
    expect(!history.modified(), "mark saved records exact history state");
    expect(history.undo(), "saved command can still be undone");
    expect(history.modified(), "undoing away from saved state marks document modified");
    expect(history.redo(), "saved command can be redone");
    expect(!history.modified(), "redoing to saved state restores clean status");
}

void test_branching_invalidates_redo_and_saved_identity() {
    anvil::editor::CommandHistory history;
    int value{};

    history.execute(std::make_unique<SetIntegerCommand>(value, 1));
    history.mark_saved();
    expect(history.undo(), "saved edit can be undone before branching");

    history.execute(std::make_unique<SetIntegerCommand>(value, 1, "Alternative edit"));
    expect(!history.can_redo(), "new edit invalidates abandoned redo branch");
    expect(history.modified(),
           "new branch remains modified even when resulting value matches saved value");
}

void test_capacity_discards_only_oldest_undo_entries() {
    anvil::editor::CommandHistory history{2};
    int value{};

    history.execute(std::make_unique<SetIntegerCommand>(value, 1));
    history.execute(std::make_unique<SetIntegerCommand>(value, 2));
    history.execute(std::make_unique<SetIntegerCommand>(value, 3));
    expect(history.undo_count() == 2, "history remains within configured capacity");
    expect(history.undo(), "newest bounded command can be undone");
    expect(history.undo(), "second bounded command can be undone");
    expect(value == 1, "oldest discarded command remains applied");
    expect(!history.undo(), "discarded command is no longer undoable");
}

void test_command_failures_preserve_stack_state() {
    anvil::editor::CommandHistory history;
    int value{};

    bool apply_threw = false;
    try {
        history.execute(std::make_unique<ThrowingCommand>(value, true, false));
    } catch (const std::runtime_error&) {
        apply_threw = true;
    }
    expect(apply_threw, "command application failure propagates");
    expect(value == 0 && !history.can_undo() && !history.modified(),
           "failed application does not enter history");

    history.execute(std::make_unique<ThrowingCommand>(value, false, true));
    bool undo_threw = false;
    try {
        static_cast<void>(history.undo());
    } catch (const std::runtime_error&) {
        undo_threw = true;
    }
    expect(undo_threw, "command undo failure propagates");
    expect(value == 1 && history.can_undo() && !history.can_redo(),
           "failed undo leaves command on undo stack");
}

} // namespace

int main() {
    test_execute_undo_redo_and_saved_state();
    test_branching_invalidates_redo_and_saved_identity();
    test_capacity_discards_only_oldest_undo_entries();
    test_command_failures_preserve_stack_state();

    if (failures == 0) {
        std::cout << "All editor command history tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
