#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace anvil::assets {

struct AssetId {
    std::uint64_t high{};
    std::uint64_t low{};

    [[nodiscard]] static AssetId generate();
    [[nodiscard]] static std::optional<AssetId> from_string(std::string_view value) noexcept;

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] constexpr bool valid() const noexcept { return high != 0 || low != 0; }

    [[nodiscard]] friend constexpr bool operator==(const AssetId&, const AssetId&) = default;
};

struct AssetIdHash {
    [[nodiscard]] std::size_t operator()(AssetId id) const noexcept;
};

} // namespace anvil::assets
