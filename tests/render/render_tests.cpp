#include <anvil/render/render_instance.hpp>

#include <iostream>
#include <string_view>
#include <utility>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failures;
    }
}

} // namespace

int main() {
    using anvil::render::AdapterType;

    expect(anvil::render::to_string(AdapterType::discrete_gpu) == "discrete_gpu",
           "adapter type has stable engine-owned text");

    anvil::render::RenderInstance instance{{
        .application_name = "Anvil render bootstrap test",
        .enable_validation = true,
        .require_validation = false,
    }};
    const auto& capabilities = instance.capabilities();
    expect(capabilities.loader_api_version.major >= 1, "Vulkan loader version is reported");
    expect(capabilities.instance_api_version.major == 1 &&
               capabilities.instance_api_version.minor == 3,
           "renderer uses the Vulkan 1.3 compatibility baseline");
    expect(!capabilities.validation_enabled || capabilities.validation_available,
           "validation is enabled only when available");

    for (const auto& adapter : capabilities.adapters) {
        expect(!adapter.name.empty(), "enumerated adapter has a name");
        expect(adapter.api_version.major >= 1, "enumerated adapter reports an API version");
    }

    anvil::render::RenderInstance move_target{{
        .application_name = "Anvil render move test",
        .enable_validation = false,
        .require_validation = false,
    }};
    move_target = std::move(instance);
    expect(move_target.capabilities().instance_api_version.minor == 3,
           "render instance preserves capabilities through move assignment");

    if (failures == 0) {
        std::cout << "Render bootstrap tests passed with " << capabilities.adapters.size()
                  << " adapter(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
