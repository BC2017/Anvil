find_package(Threads REQUIRED)

add_library(anvil_assets STATIC
    src/asset_id.cpp
    src/asset_registry.cpp
)
add_library(anvil::assets ALIAS anvil_assets)

target_include_directories(anvil_assets PUBLIC include)
target_link_libraries(anvil_assets
    PUBLIC
        anvil::core
        Threads::Threads
    PRIVATE
        anvil_project_warnings
)
set_target_properties(anvil_assets PROPERTIES FOLDER "Engine")
anvil_enable_static_analysis(anvil_assets)
