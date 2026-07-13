#include <anvil/render/scene_types.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <string_view>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failures;
    }
}

void test_material_normalization() {
    using anvil::render::DataCorrection;

    const auto unchanged = anvil::render::normalize(anvil::render::PbrMaterial{});
    expect(!unchanged.corrected(), "default PBR material is already valid");

    anvil::render::PbrMaterial invalid{};
    invalid.base_color = {-1.0F, 2.0F, std::numeric_limits<float>::quiet_NaN()};
    invalid.opacity = -0.5F;
    invalid.metallic = 2.0F;
    invalid.perceptual_roughness = 0.0F;
    invalid.emissive_color = {-1.0F, 4.0F, 2.0F};
    invalid.emissive_intensity_nits = -10.0F;
    invalid.normal_scale = 3.0F;
    invalid.occlusion_strength = -1.0F;
    invalid.alpha_cutoff = 4.0F;

    const auto normalized = anvil::render::normalize(invalid);
    expect(normalized.corrected(), "invalid PBR material reports corrections");
    expect(anvil::render::has_correction(normalized.corrections, DataCorrection::non_finite),
           "non-finite material data is diagnosed");
    expect(anvil::render::has_correction(normalized.corrections,
                                         DataCorrection::clamped_range),
           "out-of-range material data is diagnosed");
    expect(normalized.value.base_color == anvil::render::LinearColor{0.0F, 1.0F, 0.0F},
           "base color is finite and bounded");
    expect(normalized.value.metallic == 1.0F, "metalness is clamped to one");
    expect(normalized.value.perceptual_roughness >= 0.045F,
           "roughness preserves a numerically safe microfacet floor");
    expect(normalized.value.emissive_color.green == 4.0F,
           "HDR emissive color remains unbounded above");
    expect(normalized.value.emissive_intensity_nits == 0.0F,
           "negative emissive intensity is removed");
}

void test_directional_and_point_lights() {
    using anvil::render::DataCorrection;

    anvil::render::DirectionalLight directional{};
    directional.direction = {0.0F, -2.0F, 0.0F};
    const auto normalized_directional = anvil::render::normalize(directional);
    expect(normalized_directional.value.direction == anvil::render::Float3{0.0F, -1.0F, 0.0F},
           "directional light direction is normalized");
    expect(anvil::render::has_correction(normalized_directional.corrections,
                                         DataCorrection::normalized_direction),
           "direction normalization is reported");

    directional.direction = {};
    const auto invalid_directional = anvil::render::normalize(directional);
    expect(invalid_directional.value.direction == anvil::render::Float3{0.0F, -1.0F, 0.0F},
           "zero direction receives a stable fallback");
    expect(anvil::render::has_correction(invalid_directional.corrections,
                                         DataCorrection::invalid_direction),
           "invalid direction is diagnosed");

    anvil::render::PointLight point{};
    point.position.x = std::numeric_limits<float>::infinity();
    point.color.red = -1.0F;
    point.luminous_flux_lumens = -100.0F;
    point.range_meters = 0.0F;
    const auto normalized_point = anvil::render::normalize(point);
    expect(normalized_point.value.position.x == 0.0F, "non-finite position is sanitized");
    expect(normalized_point.value.color.red == 0.0F, "light color cannot be negative");
    expect(normalized_point.value.luminous_flux_lumens == 0.0F,
           "luminous flux cannot be negative");
    expect(normalized_point.value.range_meters > 0.0F, "light range remains positive");
}

void test_spot_light_angles() {
    using anvil::render::DataCorrection;

    anvil::render::SpotLight spot{};
    spot.inner_cone_radians = 1.2F;
    spot.outer_cone_radians = 0.4F;
    const auto normalized = anvil::render::normalize(spot);
    expect(normalized.value.inner_cone_radians == normalized.value.outer_cone_radians,
           "spot inner cone cannot exceed outer cone");
    expect(anvil::render::has_correction(normalized.corrections,
                                         DataCorrection::reordered_angles),
           "spot cone reordering is diagnosed");

    spot.outer_cone_radians = 10.0F;
    const auto clamped = anvil::render::normalize(spot);
    expect(clamped.value.outer_cone_radians <= 1.570'797F,
           "spot outer cone cannot exceed ninety degrees");
}

} // namespace

int main() {
    test_material_normalization();
    test_directional_and_point_lights();
    test_spot_light_angles();

    if (failures == 0) {
        std::cout << "All render data tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
