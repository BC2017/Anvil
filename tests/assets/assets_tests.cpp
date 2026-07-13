#include <anvil/assets/asset_id.hpp>
#include <anvil/assets/asset_registry.hpp>

#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failures;
    }
}

[[nodiscard]] anvil::assets::AssetMetadata metadata(const anvil::assets::AssetId id,
                                                     std::string path,
                                                     const std::uint64_t revision = 1) {
    return {
        .id = id,
        .logical_path = std::move(path),
        .type = "texture",
        .importer = "image",
        .source_revision = revision,
    };
}

void test_asset_id_round_trip() {
    const auto id = anvil::assets::AssetId::generate();
    const auto text = id.to_string();
    const auto parsed = anvil::assets::AssetId::from_string(text);

    expect(id.valid(), "generated asset ID is non-null");
    expect(text.size() == 36, "asset ID uses canonical UUID text length");
    expect(text[14] == '4', "generated asset ID carries UUID version 4 bits");
    expect(text[19] == '8' || text[19] == '9' || text[19] == 'a' || text[19] == 'b',
           "generated asset ID carries the RFC UUID variant bits");
    expect(parsed.has_value() && *parsed == id, "asset ID survives text round trip");
    expect(!anvil::assets::AssetId::from_string("not-an-asset-id").has_value(),
           "malformed asset ID is rejected");
}

void test_registry_identity_and_paths() {
    using anvil::assets::AssetId;
    using anvil::assets::AssetRegistry;
    using anvil::assets::RegistrationResult;

    AssetRegistry registry;
    const auto texture_id = AssetId::generate();
    const auto other_id = AssetId::generate();

    expect(registry.register_asset(metadata(texture_id, "textures\\environment/sky.ktx2")) ==
               RegistrationResult::inserted,
           "asset is inserted");
    expect(registry.size() == 1, "registry tracks inserted asset count");

    const auto by_path = registry.find_by_path("textures/environment/sky.ktx2");
    expect(by_path.has_value() && by_path->id == texture_id,
           "registry normalizes portable logical paths");

    expect(registry.register_asset(metadata(texture_id, "textures/environment/sky.ktx2", 2)) ==
               RegistrationResult::updated,
           "same stable identity updates metadata");
    expect(registry.find(texture_id)->source_revision == 2,
           "updated metadata replaces the previous revision");

    expect(registry.register_asset(metadata(texture_id, "textures/other.ktx2")) ==
               RegistrationResult::id_conflict,
           "one asset ID cannot identify two paths");
    expect(registry.register_asset(metadata(other_id, "textures/environment/sky.ktx2")) ==
               RegistrationResult::path_conflict,
           "one logical path cannot identify two assets");
    expect(registry.register_asset(metadata({}, "textures/null.ktx2")) ==
               RegistrationResult::invalid_metadata,
           "null asset ID is rejected");
    expect(registry.register_asset(metadata(other_id, "../outside.ktx2")) ==
               RegistrationResult::invalid_metadata,
           "logical path cannot escape the asset root");

    expect(registry.remove(texture_id), "registered asset can be removed");
    expect(!registry.find(texture_id).has_value(), "removed asset is no longer addressable");
    expect(!registry.find_by_path("textures/environment/sky.ktx2").has_value(),
           "removal clears the path index");
}

void test_concurrent_registry_reads_and_updates() {
    using anvil::assets::AssetId;
    using anvil::assets::AssetRegistry;
    using anvil::assets::RegistrationResult;

    AssetRegistry registry;
    const auto id = AssetId::generate();
    expect(registry.register_asset(metadata(id, "textures/concurrent.ktx2")) ==
               RegistrationResult::inserted,
           "concurrency fixture is inserted");

    std::atomic_bool healthy{true};
    std::vector<std::thread> readers;
    readers.reserve(4);
    for (int thread_index = 0; thread_index < 4; ++thread_index) {
        readers.emplace_back([&] {
            for (int iteration = 0; iteration < 1'000; ++iteration) {
                const auto asset = registry.find(id);
                if (!asset.has_value() || asset->id != id) {
                    healthy.store(false, std::memory_order_relaxed);
                }
            }
        });
    }

    for (std::uint64_t revision = 2; revision < 100; ++revision) {
        if (registry.register_asset(metadata(id, "textures/concurrent.ktx2", revision)) !=
            RegistrationResult::updated) {
            healthy.store(false, std::memory_order_relaxed);
        }
    }
    for (auto& reader : readers) {
        reader.join();
    }
    expect(healthy.load(std::memory_order_relaxed),
           "concurrent reads and metadata updates preserve registry invariants");
}

} // namespace

int main() {
    test_asset_id_round_trip();
    test_registry_identity_and_paths();
    test_concurrent_registry_reads_and_updates();

    if (failures == 0) {
        std::cout << "All asset tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
