#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace anvil::editor {

class EditorCommand {
  public:
    virtual ~EditorCommand() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    virtual void apply() = 0;
    virtual void undo() = 0;
};

class CommandHistory final {
  public:
    explicit CommandHistory(std::size_t capacity = 256);

    void execute(std::unique_ptr<EditorCommand> command);
    [[nodiscard]] bool undo();
    [[nodiscard]] bool redo();

    void mark_saved() noexcept;
    void reset() noexcept;

    [[nodiscard]] bool can_undo() const noexcept;
    [[nodiscard]] bool can_redo() const noexcept;
    [[nodiscard]] bool modified() const noexcept;
    [[nodiscard]] std::optional<std::string_view> undo_name() const noexcept;
    [[nodiscard]] std::optional<std::string_view> redo_name() const noexcept;
    [[nodiscard]] std::size_t undo_count() const noexcept;
    [[nodiscard]] std::size_t redo_count() const noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;

  private:
    struct Entry {
        std::unique_ptr<EditorCommand> command;
        std::uint64_t before_state{};
        std::uint64_t after_state{};
    };

    [[nodiscard]] std::uint64_t allocate_state() noexcept;

    std::size_t capacity_{};
    std::vector<Entry> undo_stack_;
    std::vector<Entry> redo_stack_;
    std::uint64_t current_state_{};
    std::uint64_t saved_state_{};
    std::uint64_t next_state_{1};
};

} // namespace anvil::editor
