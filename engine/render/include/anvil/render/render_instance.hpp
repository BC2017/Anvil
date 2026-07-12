#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace anvil::render {

struct ApiVersion {
    std::uint32_t major{};
    std::uint32_t minor{};
    std::uint32_t patch{};

    [[nodiscard]] friend constexpr bool operator==(const ApiVersion&, const ApiVersion&) = default;
};

enum class AdapterType { other, integrated_gpu, discrete_gpu, virtual_gpu, cpu };

[[nodiscard]] constexpr std::string_view to_string(const AdapterType type) noexcept {
    switch (type) {
    case AdapterType::other:
        return "other";
    case AdapterType::integrated_gpu:
        return "integrated_gpu";
    case AdapterType::discrete_gpu:
        return "discrete_gpu";
    case AdapterType::virtual_gpu:
        return "virtual_gpu";
    case AdapterType::cpu:
        return "cpu";
    }
    return "unknown";
}

struct AdapterInfo {
    std::string name;
    AdapterType type{AdapterType::other};
    ApiVersion api_version{};
    std::uint32_t vendor_id{};
    std::uint32_t device_id{};
    std::uint64_t device_local_memory_bytes{};
};

struct RenderCapabilities {
    ApiVersion loader_api_version{};
    ApiVersion instance_api_version{};
    bool validation_available{};
    bool validation_enabled{};
    bool debug_utilities_available{};
    std::vector<AdapterInfo> adapters;
};

struct RenderInstanceConfig {
    std::string application_name{"Anvil"};
    bool enable_validation{true};
    bool require_validation{false};
};

class RenderInstance final {
  public:
    explicit RenderInstance(RenderInstanceConfig config = {});
    ~RenderInstance();

    RenderInstance(const RenderInstance&) = delete;
    RenderInstance& operator=(const RenderInstance&) = delete;
    RenderInstance(RenderInstance&&) noexcept;
    RenderInstance& operator=(RenderInstance&&) noexcept;

    [[nodiscard]] const RenderCapabilities& capabilities() const noexcept;

  private:
    struct State;
    std::unique_ptr<State> state_;
};

} // namespace anvil::render
