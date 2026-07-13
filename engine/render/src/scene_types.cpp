#include <anvil/render/scene_types.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace anvil::render {
namespace {

constexpr float minimum_roughness = 0.045F;
constexpr float minimum_light_range = 0.01F;
constexpr float maximum_spot_angle = 1.570'796'326F;

void sanitize_finite(float& value, const float fallback, DataCorrection& corrections) noexcept {
    if (!std::isfinite(value)) {
        value = fallback;
        corrections |= DataCorrection::non_finite;
    }
}

void clamp_value(float& value, const float minimum, const float maximum,
                 DataCorrection& corrections) noexcept {
    const float clamped = std::clamp(value, minimum, maximum);
    if (clamped != value) {
        value = clamped;
        corrections |= DataCorrection::clamped_range;
    }
}

void sanitize_bounded(float& value, const float fallback, const float minimum,
                      const float maximum, DataCorrection& corrections) noexcept {
    sanitize_finite(value, fallback, corrections);
    clamp_value(value, minimum, maximum, corrections);
}

void sanitize_nonnegative(float& value, const float fallback,
                          DataCorrection& corrections) noexcept {
    sanitize_finite(value, fallback, corrections);
    clamp_value(value, 0.0F, std::numeric_limits<float>::max(), corrections);
}

void sanitize_color(LinearColor& color, const bool bounded,
                    DataCorrection& corrections) noexcept {
    auto sanitize_channel = [&](float& channel) {
        if (bounded) {
            sanitize_bounded(channel, 0.0F, 0.0F, 1.0F, corrections);
        } else {
            sanitize_nonnegative(channel, 0.0F, corrections);
        }
    };
    sanitize_channel(color.red);
    sanitize_channel(color.green);
    sanitize_channel(color.blue);
}

void sanitize_position(Float3& position, DataCorrection& corrections) noexcept {
    sanitize_finite(position.x, 0.0F, corrections);
    sanitize_finite(position.y, 0.0F, corrections);
    sanitize_finite(position.z, 0.0F, corrections);
}

void normalize_direction(Float3& direction, DataCorrection& corrections) noexcept {
    if (!std::isfinite(direction.x) || !std::isfinite(direction.y) ||
        !std::isfinite(direction.z)) {
        direction = {0.0F, -1.0F, 0.0F};
        corrections |= DataCorrection::non_finite;
        corrections |= DataCorrection::invalid_direction;
        return;
    }

    const float length_squared =
        direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;
    if (length_squared <= std::numeric_limits<float>::epsilon()) {
        direction = {0.0F, -1.0F, 0.0F};
        corrections |= DataCorrection::invalid_direction;
        return;
    }

    const float length = std::sqrt(length_squared);
    if (std::abs(length - 1.0F) > 0.000'01F) {
        const float inverse_length = 1.0F / length;
        direction.x *= inverse_length;
        direction.y *= inverse_length;
        direction.z *= inverse_length;
        corrections |= DataCorrection::normalized_direction;
    }
}

void sanitize_light_common(LinearColor& color, float& luminous_value, float* range,
                           DataCorrection& corrections) noexcept {
    sanitize_color(color, false, corrections);
    sanitize_nonnegative(luminous_value, 0.0F, corrections);
    if (range != nullptr) {
        sanitize_finite(*range, minimum_light_range, corrections);
        clamp_value(*range, minimum_light_range, std::numeric_limits<float>::max(), corrections);
    }
}

} // namespace

NormalizationResult<PbrMaterial> normalize(PbrMaterial material) noexcept {
    DataCorrection corrections{DataCorrection::none};
    sanitize_color(material.base_color, true, corrections);
    sanitize_bounded(material.opacity, 1.0F, 0.0F, 1.0F, corrections);
    sanitize_bounded(material.metallic, 0.0F, 0.0F, 1.0F, corrections);
    sanitize_bounded(material.perceptual_roughness, 1.0F, minimum_roughness, 1.0F,
                     corrections);
    sanitize_color(material.emissive_color, false, corrections);
    sanitize_nonnegative(material.emissive_intensity_nits, 0.0F, corrections);
    sanitize_bounded(material.normal_scale, 1.0F, 0.0F, 2.0F, corrections);
    sanitize_bounded(material.occlusion_strength, 1.0F, 0.0F, 1.0F, corrections);
    sanitize_bounded(material.alpha_cutoff, 0.5F, 0.0F, 1.0F, corrections);
    return {.value = material, .corrections = corrections};
}

NormalizationResult<DirectionalLight> normalize(DirectionalLight light) noexcept {
    DataCorrection corrections{DataCorrection::none};
    normalize_direction(light.direction, corrections);
    sanitize_light_common(light.color, light.illuminance_lux, nullptr, corrections);
    return {.value = light, .corrections = corrections};
}

NormalizationResult<PointLight> normalize(PointLight light) noexcept {
    DataCorrection corrections{DataCorrection::none};
    sanitize_position(light.position, corrections);
    sanitize_light_common(light.color, light.luminous_flux_lumens, &light.range_meters,
                          corrections);
    return {.value = light, .corrections = corrections};
}

NormalizationResult<SpotLight> normalize(SpotLight light) noexcept {
    DataCorrection corrections{DataCorrection::none};
    sanitize_position(light.position, corrections);
    normalize_direction(light.direction, corrections);
    sanitize_light_common(light.color, light.luminous_flux_lumens, &light.range_meters,
                          corrections);
    sanitize_bounded(light.inner_cone_radians, 0.0F, 0.0F, maximum_spot_angle, corrections);
    sanitize_bounded(light.outer_cone_radians, 0.78F, 0.0F, maximum_spot_angle, corrections);
    if (light.inner_cone_radians > light.outer_cone_radians) {
        light.inner_cone_radians = light.outer_cone_radians;
        corrections |= DataCorrection::reordered_angles;
    }
    return {.value = light, .corrections = corrections};
}

} // namespace anvil::render
