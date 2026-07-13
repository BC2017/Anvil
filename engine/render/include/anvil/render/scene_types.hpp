#pragma once

#include <cstdint>

namespace anvil::render {

struct Float3 {
    float x{};
    float y{};
    float z{};

    [[nodiscard]] friend constexpr bool operator==(const Float3&, const Float3&) = default;
};

struct LinearColor {
    float red{};
    float green{};
    float blue{};

    [[nodiscard]] friend constexpr bool operator==(const LinearColor&, const LinearColor&) = default;
};

enum class AlphaMode { opaque, mask, blend };

struct PbrMaterial {
    LinearColor base_color{1.0F, 1.0F, 1.0F};
    float opacity{1.0F};
    float metallic{};
    float perceptual_roughness{1.0F};
    LinearColor emissive_color{};
    float emissive_intensity_nits{};
    float normal_scale{1.0F};
    float occlusion_strength{1.0F};
    AlphaMode alpha_mode{AlphaMode::opaque};
    float alpha_cutoff{0.5F};
    bool double_sided{};
};

struct DirectionalLight {
    Float3 direction{0.0F, -1.0F, 0.0F};
    LinearColor color{1.0F, 1.0F, 1.0F};
    float illuminance_lux{100'000.0F};
};

struct PointLight {
    Float3 position{};
    LinearColor color{1.0F, 1.0F, 1.0F};
    float luminous_flux_lumens{1'000.0F};
    float range_meters{10.0F};
};

struct SpotLight {
    Float3 position{};
    Float3 direction{0.0F, -1.0F, 0.0F};
    LinearColor color{1.0F, 1.0F, 1.0F};
    float luminous_flux_lumens{1'000.0F};
    float range_meters{10.0F};
    float inner_cone_radians{0.35F};
    float outer_cone_radians{0.78F};
};

enum class DataCorrection : std::uint32_t {
    none = 0,
    non_finite = 1U << 0U,
    clamped_range = 1U << 1U,
    normalized_direction = 1U << 2U,
    invalid_direction = 1U << 3U,
    reordered_angles = 1U << 4U,
};

[[nodiscard]] constexpr DataCorrection operator|(const DataCorrection left,
                                                 const DataCorrection right) noexcept {
    return static_cast<DataCorrection>(static_cast<std::uint32_t>(left) |
                                       static_cast<std::uint32_t>(right));
}

constexpr DataCorrection& operator|=(DataCorrection& left, const DataCorrection right) noexcept {
    left = left | right;
    return left;
}

[[nodiscard]] constexpr bool has_correction(const DataCorrection value,
                                            const DataCorrection correction) noexcept {
    return (static_cast<std::uint32_t>(value) & static_cast<std::uint32_t>(correction)) != 0U;
}

template <typename Value>
struct NormalizationResult {
    Value value;
    DataCorrection corrections{DataCorrection::none};

    [[nodiscard]] constexpr bool corrected() const noexcept {
        return corrections != DataCorrection::none;
    }
};

[[nodiscard]] NormalizationResult<PbrMaterial> normalize(PbrMaterial material) noexcept;
[[nodiscard]] NormalizationResult<DirectionalLight> normalize(DirectionalLight light) noexcept;
[[nodiscard]] NormalizationResult<PointLight> normalize(PointLight light) noexcept;
[[nodiscard]] NormalizationResult<SpotLight> normalize(SpotLight light) noexcept;

} // namespace anvil::render
