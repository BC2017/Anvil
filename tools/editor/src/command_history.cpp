#include <anvil/editor/command_history.hpp>

#include <limits>
#include <stdexcept>
#include <utility>

namespace anvil::editor {

CommandHistory::CommandHistory(const std::size_t capacity) : capacity_{capacity} {
    if (capacity_ == 0) {
        throw std::invalid_argument{"Command history capacity must be positive"};
    }
    undo_stack_.reserve(capacity_);
    redo_stack_.reserve(capacity_);
}

void CommandHistory::execute(std::unique_ptr<EditorCommand> command) {
    if (command == nullptr) {
        throw std::invalid_argument{"Cannot execute a null editor command"};
    }

    command->apply();
    redo_stack_.clear();

    Entry entry{
        .command = std::move(command),
        .before_state = current_state_,
        .after_state = allocate_state(),
    };
    current_state_ = entry.after_state;

    if (undo_stack_.size() == capacity_) {
        undo_stack_.erase(undo_stack_.begin());
    }
    undo_stack_.push_back(std::move(entry));
}

bool CommandHistory::undo() {
    if (undo_stack_.empty()) {
        return false;
    }

    auto& entry = undo_stack_.back();
    entry.command->undo();
    current_state_ = entry.before_state;
    redo_stack_.push_back(std::move(entry));
    undo_stack_.pop_back();
    return true;
}

bool CommandHistory::redo() {
    if (redo_stack_.empty()) {
        return false;
    }

    auto& entry = redo_stack_.back();
    entry.command->apply();
    current_state_ = entry.after_state;
    undo_stack_.push_back(std::move(entry));
    redo_stack_.pop_back();
    return true;
}

void CommandHistory::mark_saved() noexcept {
    saved_state_ = current_state_;
}

void CommandHistory::reset() noexcept {
    undo_stack_.clear();
    redo_stack_.clear();
    current_state_ = allocate_state();
    saved_state_ = current_state_;
}

bool CommandHistory::can_undo() const noexcept {
    return !undo_stack_.empty();
}

bool CommandHistory::can_redo() const noexcept {
    return !redo_stack_.empty();
}

bool CommandHistory::modified() const noexcept {
    return current_state_ != saved_state_;
}

std::optional<std::string_view> CommandHistory::undo_name() const noexcept {
    return undo_stack_.empty() ? std::nullopt
                               : std::optional{undo_stack_.back().command->name()};
}

std::optional<std::string_view> CommandHistory::redo_name() const noexcept {
    return redo_stack_.empty() ? std::nullopt
                               : std::optional{redo_stack_.back().command->name()};
}

std::size_t CommandHistory::undo_count() const noexcept {
    return undo_stack_.size();
}

std::size_t CommandHistory::redo_count() const noexcept {
    return redo_stack_.size();
}

std::size_t CommandHistory::capacity() const noexcept {
    return capacity_;
}

std::uint64_t CommandHistory::allocate_state() noexcept {
    const auto state = next_state_++;
    if (next_state_ == 0) {
        next_state_ = 1;
    }
    return state;
}

} // namespace anvil::editor
