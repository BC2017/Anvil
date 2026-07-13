#include <anvil/assets/asset_id.hpp>

#include <array>
#include <random>

namespace anvil::assets {
namespace {

constexpr std::array hyphen_positions{8U, 13U, 18U, 23U};
constexpr std::string_view hex_digits{"0123456789abcdef"};

[[nodiscard]] constexpr int hex_value(const char character) noexcept {
    if (character >= '0' && character <= '9') {
        return character - '0';
    }
    if (character >= 'a' && character <= 'f') {
        return character - 'a' + 10;
    }
    if (character >= 'A' && character <= 'F') {
        return character - 'A' + 10;
    }
    return -1;
}

[[nodiscard]] std::mt19937_64& random_generator() {
    thread_local std::mt19937_64 generator{[] {
        std::random_device device;
        std::array<std::uint32_t, 8> seed_data{};
        for (auto& value : seed_data) {
            value = device();
        }
        std::seed_seq seed{seed_data.begin(), seed_data.end()};
        return std::mt19937_64{seed};
    }()};
    return generator;
}

[[nodiscard]] constexpr bool is_hyphen_position(const std::size_t index) noexcept {
    for (const auto position : hyphen_positions) {
        if (position == index) {
            return true;
        }
    }
    return false;
}

} // namespace

AssetId AssetId::generate() {
    auto& generator = random_generator();
    AssetId id{.high = generator(), .low = generator()};

    id.high = (id.high & ~(std::uint64_t{0xF} << 12U)) | (std::uint64_t{0x4} << 12U);
    id.low = (id.low & ~(std::uint64_t{0x3} << 62U)) | (std::uint64_t{0x2} << 62U);
    return id;
}

std::optional<AssetId> AssetId::from_string(const std::string_view value) noexcept {
    if (value.size() != 36) {
        return std::nullopt;
    }

    std::array<std::uint8_t, 16> bytes{};
    std::size_t byte_index{};
    for (std::size_t index = 0; index < value.size();) {
        if (is_hyphen_position(index)) {
            if (value[index] != '-') {
                return std::nullopt;
            }
            ++index;
            continue;
        }
        if (index + 1 >= value.size() || byte_index >= bytes.size()) {
            return std::nullopt;
        }

        const int high_nibble = hex_value(value[index]);
        const int low_nibble = hex_value(value[index + 1]);
        if (high_nibble < 0 || low_nibble < 0) {
            return std::nullopt;
        }
        bytes[byte_index++] =
            static_cast<std::uint8_t>((high_nibble << 4) | low_nibble);
        index += 2;
    }
    if (byte_index != bytes.size()) {
        return std::nullopt;
    }

    AssetId id{};
    for (std::size_t index = 0; index < 8; ++index) {
        id.high = (id.high << 8U) | bytes[index];
        id.low = (id.low << 8U) | bytes[index + 8];
    }
    return id;
}

std::string AssetId::to_string() const {
    std::array<std::uint8_t, 16> bytes{};
    for (std::size_t index = 0; index < 8; ++index) {
        const auto shift = static_cast<unsigned>((7U - index) * 8U);
        bytes[index] = static_cast<std::uint8_t>(high >> shift);
        bytes[index + 8] = static_cast<std::uint8_t>(low >> shift);
    }

    std::string result;
    result.reserve(36);
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        if (index == 4 || index == 6 || index == 8 || index == 10) {
            result.push_back('-');
        }
        result.push_back(hex_digits[bytes[index] >> 4U]);
        result.push_back(hex_digits[bytes[index] & 0xFU]);
    }
    return result;
}

std::size_t AssetIdHash::operator()(const AssetId id) const noexcept {
    const auto mixed = id.high ^ (id.low + 0x9e3779b97f4a7c15ULL + (id.high << 6U) +
                                  (id.high >> 2U));
    return static_cast<std::size_t>(mixed);
}

} // namespace anvil::assets
