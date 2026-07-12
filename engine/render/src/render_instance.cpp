#include <anvil/render/render_instance.hpp>

#include <anvil/core/log.hpp>

#include <vulkan/vulkan.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace anvil::render {
namespace {

constexpr std::string_view validation_layer_name{"VK_LAYER_KHRONOS_validation"};

[[nodiscard]] ApiVersion convert_version(const std::uint32_t version) noexcept {
    return {
        .major = VK_API_VERSION_MAJOR(version),
        .minor = VK_API_VERSION_MINOR(version),
        .patch = VK_API_VERSION_PATCH(version),
    };
}

[[nodiscard]] AdapterType convert_adapter_type(const VkPhysicalDeviceType type) noexcept {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return AdapterType::integrated_gpu;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return AdapterType::discrete_gpu;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return AdapterType::virtual_gpu;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return AdapterType::cpu;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    default:
        return AdapterType::other;
    }
}

void require_success(const VkResult result, const std::string_view operation) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error{std::string{operation} + " failed with Vulkan result " +
                                 std::to_string(result)};
    }
}

[[nodiscard]] std::vector<VkLayerProperties> enumerate_layers() {
    std::uint32_t count{};
    require_success(vkEnumerateInstanceLayerProperties(&count, nullptr),
                    "vkEnumerateInstanceLayerProperties");
    std::vector<VkLayerProperties> layers(count);
    require_success(vkEnumerateInstanceLayerProperties(&count, layers.data()),
                    "vkEnumerateInstanceLayerProperties");
    layers.resize(count);
    return layers;
}

[[nodiscard]] std::vector<VkExtensionProperties> enumerate_extensions() {
    std::uint32_t count{};
    require_success(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr),
                    "vkEnumerateInstanceExtensionProperties");
    std::vector<VkExtensionProperties> extensions(count);
    require_success(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()),
                    "vkEnumerateInstanceExtensionProperties");
    extensions.resize(count);
    return extensions;
}

template <typename Properties, typename NameAccessor>
[[nodiscard]] bool contains_name(const std::vector<Properties>& properties,
                                 const std::string_view required_name,
                                 NameAccessor&& name_accessor) {
    return std::ranges::any_of(properties, [&](const Properties& property) {
        return std::string_view{name_accessor(property)} == required_name;
    });
}

[[nodiscard]] std::uint32_t query_loader_version() {
    const auto enumerate_version = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
        vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
    if (enumerate_version == nullptr) {
        return VK_API_VERSION_1_0;
    }

    std::uint32_t version{VK_API_VERSION_1_0};
    require_success(enumerate_version(&version), "vkEnumerateInstanceVersion");
    return version;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                               VkDebugUtilsMessageTypeFlagsEXT,
                                               const VkDebugUtilsMessengerCallbackDataEXT* data,
                                               void*) {
    auto level = core::LogLevel::debug;
    if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0U) {
        level = core::LogLevel::error;
    } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0U) {
        level = core::LogLevel::warning;
    } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0U) {
        level = core::LogLevel::info;
    }

    const char* message = data != nullptr ? data->pMessage : nullptr;
    core::log(level, "vulkan", message != nullptr ? message : "Unknown validation message");
    return VK_FALSE;
}

[[nodiscard]] VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info() noexcept {
    return {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
        .pUserData = nullptr,
    };
}

[[nodiscard]] std::vector<AdapterInfo> enumerate_adapters(const VkInstance instance) {
    std::uint32_t device_count{};
    require_success(vkEnumeratePhysicalDevices(instance, &device_count, nullptr),
                    "vkEnumeratePhysicalDevices");
    std::vector<VkPhysicalDevice> devices(device_count);
    require_success(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()),
                    "vkEnumeratePhysicalDevices");
    devices.resize(device_count);

    std::vector<AdapterInfo> adapters;
    adapters.reserve(devices.size());
    for (const auto device : devices) {
        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceMemoryProperties memory_properties{};
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

        std::uint64_t device_local_memory{};
        for (std::uint32_t index = 0; index < memory_properties.memoryHeapCount; ++index) {
            const auto& heap = memory_properties.memoryHeaps[index];
            if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0U) {
                device_local_memory += heap.size;
            }
        }

        adapters.push_back({
            .name = properties.deviceName,
            .type = convert_adapter_type(properties.deviceType),
            .api_version = convert_version(properties.apiVersion),
            .vendor_id = properties.vendorID,
            .device_id = properties.deviceID,
            .device_local_memory_bytes = device_local_memory,
        });
    }
    return adapters;
}

} // namespace

struct RenderInstance::State {
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debug_messenger{VK_NULL_HANDLE};
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger{};
    RenderCapabilities capabilities;

    ~State() {
        if (debug_messenger != VK_NULL_HANDLE && destroy_debug_messenger != nullptr) {
            destroy_debug_messenger(instance, debug_messenger, nullptr);
        }
        if (instance != VK_NULL_HANDLE) {
            vkDestroyInstance(instance, nullptr);
        }
    }
};

RenderInstance::RenderInstance(RenderInstanceConfig config) : state_{std::make_unique<State>()} {
    const auto loader_version = query_loader_version();
    if (loader_version < VK_API_VERSION_1_3) {
        throw std::runtime_error{"Anvil requires a Vulkan 1.3 loader"};
    }

    const auto layers = enumerate_layers();
    const auto extensions = enumerate_extensions();
    const bool validation_available = contains_name(
        layers, validation_layer_name, [](const VkLayerProperties& layer) { return layer.layerName; });
    const bool debug_utilities_available = contains_name(
        extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        [](const VkExtensionProperties& extension) { return extension.extensionName; });

    if (config.require_validation && !validation_available) {
        throw std::runtime_error{"Vulkan validation was required but is not available"};
    }
    const bool validation_enabled = config.enable_validation && validation_available;
    if (config.enable_validation && !validation_available) {
        core::log(core::LogLevel::warning, "vulkan",
                  "Validation was requested but VK_LAYER_KHRONOS_validation is unavailable");
    }

    const std::array validation_layers{validation_layer_name.data()};
    const std::array debug_extensions{VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    auto debug_create_info = debug_messenger_create_info();
    const VkApplicationInfo application_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = config.application_name.c_str(),
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "Anvil",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };
    const bool enable_debug_messenger = validation_enabled && debug_utilities_available;
    const VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = enable_debug_messenger ? &debug_create_info : nullptr,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = validation_enabled ? 1U : 0U,
        .ppEnabledLayerNames = validation_enabled ? validation_layers.data() : nullptr,
        .enabledExtensionCount = enable_debug_messenger ? 1U : 0U,
        .ppEnabledExtensionNames = enable_debug_messenger ? debug_extensions.data() : nullptr,
    };
    require_success(vkCreateInstance(&create_info, nullptr, &state_->instance), "vkCreateInstance");

    if (enable_debug_messenger) {
        const auto create_debug_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(state_->instance, "vkCreateDebugUtilsMessengerEXT"));
        state_->destroy_debug_messenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(state_->instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (create_debug_messenger != nullptr && state_->destroy_debug_messenger != nullptr) {
            require_success(create_debug_messenger(state_->instance, &debug_create_info, nullptr,
                                                   &state_->debug_messenger),
                            "vkCreateDebugUtilsMessengerEXT");
        }
    }

    state_->capabilities = {
        .loader_api_version = convert_version(loader_version),
        .instance_api_version = convert_version(VK_API_VERSION_1_3),
        .validation_available = validation_available,
        .validation_enabled = validation_enabled,
        .debug_utilities_available = debug_utilities_available,
        .adapters = enumerate_adapters(state_->instance),
    };

    core::log(core::LogLevel::info, "vulkan",
              "Vulkan instance initialized with " +
                  std::to_string(state_->capabilities.adapters.size()) + " adapter(s)");
}

RenderInstance::~RenderInstance() = default;

RenderInstance::RenderInstance(RenderInstance&&) noexcept = default;

RenderInstance& RenderInstance::operator=(RenderInstance&&) noexcept = default;

const RenderCapabilities& RenderInstance::capabilities() const noexcept {
    return state_->capabilities;
}

} // namespace anvil::render
