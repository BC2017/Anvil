add_library(anvil_platform STATIC
    src/application.cpp
)
add_library(anvil::platform ALIAS anvil_platform)

target_include_directories(anvil_platform PUBLIC include)
target_link_libraries(anvil_platform
    PUBLIC
        anvil::core
        SDL3::SDL3
    PRIVATE anvil_project_warnings
)
set_target_properties(anvil_platform PROPERTIES FOLDER "Engine")
anvil_enable_static_analysis(anvil_platform)
