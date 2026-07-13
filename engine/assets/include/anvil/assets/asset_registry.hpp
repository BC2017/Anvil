#pragma once

#include <anvil/assets/asset_id.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace anvil::assets {

struct AssetMetadata {
    AssetId id;
    std::string logical_path;
    std::string type;
    std::string importer;
    std::uint64_t source_revision{};

    [[nodiscard]] friend bool operator==(const AssetMetadata&, const AssetMetadata&) = default;
};

enum class RegistrationResult { inserted, updated, invalid_metadata, id_conflict, path_conflict };

[[nodiscard]] constexpr std::string_view to_string(const RegistrationResult result) noexcept {
    switch (result) {
    case RegistrationResult::inserted:
        return "inserted";
    case RegistrationResult::updated:
        return "updated";
    case RegistrationResult::invalid_metadata:
        return "invalid_metadata";
    case RegistrationResult::id_conflict:
        return "id_conflict";
    case RegistrationResult::path_conflict:
        return "path_conflict";
    }
    return "unknown";
}

class AssetRegistry final {
  public:
    [[nodiscard]] RegistrationResult register_asset(AssetMetadata metadata);
    [[nodiscard]] bool remove(AssetId id);

    [[nodiscard]] std::optional<AssetMetadata> find(AssetId id) const;
    [[nodiscard]] std::optional<AssetMetadata> find_by_path(std::string_view logical_path) const;
    [[nodiscard]] std::size_t size() const;

  private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<AssetId, AssetMetadata, AssetIdHash> assets_;
    std::unordered_map<std::string, AssetId> paths_;
};

} // namespace anvil::assets
