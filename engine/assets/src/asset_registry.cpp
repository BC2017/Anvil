#include <anvil/assets/asset_registry.hpp>

#include <filesystem>
#include <mutex>
#include <utility>

namespace anvil::assets {
namespace {

[[nodiscard]] std::optional<std::string> normalize_logical_path(const std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }

    std::string portable_path{value};
    for (auto& character : portable_path) {
        if (character == '\\') {
            character = '/';
        }
    }

    const std::filesystem::path path{portable_path};
    if (path.is_absolute() || path.has_root_name()) {
        return std::nullopt;
    }
    const auto normalized = path.lexically_normal().generic_string();
    if (normalized.empty() || normalized == "." || normalized.starts_with("../") ||
        normalized == "..") {
        return std::nullopt;
    }
    return normalized;
}

} // namespace

RegistrationResult AssetRegistry::register_asset(AssetMetadata metadata) {
    const auto normalized_path = normalize_logical_path(metadata.logical_path);
    if (!metadata.id.valid() || !normalized_path.has_value() || metadata.type.empty() ||
        metadata.importer.empty()) {
        return RegistrationResult::invalid_metadata;
    }
    metadata.logical_path = *normalized_path;

    const std::unique_lock lock{mutex_};
    const auto id_entry = assets_.find(metadata.id);
    if (id_entry != assets_.end()) {
        if (id_entry->second.logical_path != metadata.logical_path) {
            return RegistrationResult::id_conflict;
        }
        id_entry->second = std::move(metadata);
        return RegistrationResult::updated;
    }

    const auto path_entry = paths_.find(metadata.logical_path);
    if (path_entry != paths_.end() && path_entry->second != metadata.id) {
        return RegistrationResult::path_conflict;
    }

    paths_.emplace(metadata.logical_path, metadata.id);
    assets_.emplace(metadata.id, std::move(metadata));
    return RegistrationResult::inserted;
}

bool AssetRegistry::remove(const AssetId id) {
    const std::unique_lock lock{mutex_};
    const auto entry = assets_.find(id);
    if (entry == assets_.end()) {
        return false;
    }
    paths_.erase(entry->second.logical_path);
    assets_.erase(entry);
    return true;
}

std::optional<AssetMetadata> AssetRegistry::find(const AssetId id) const {
    const std::shared_lock lock{mutex_};
    const auto entry = assets_.find(id);
    return entry != assets_.end() ? std::optional{entry->second} : std::nullopt;
}

std::optional<AssetMetadata> AssetRegistry::find_by_path(const std::string_view logical_path) const {
    const auto normalized_path = normalize_logical_path(logical_path);
    if (!normalized_path.has_value()) {
        return std::nullopt;
    }

    const std::shared_lock lock{mutex_};
    const auto path_entry = paths_.find(*normalized_path);
    if (path_entry == paths_.end()) {
        return std::nullopt;
    }
    return assets_.at(path_entry->second);
}

std::size_t AssetRegistry::size() const {
    const std::shared_lock lock{mutex_};
    return assets_.size();
}

} // namespace anvil::assets
