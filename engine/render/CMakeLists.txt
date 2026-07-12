add_library(anvil_render STATIC
    src/render_instance.cpp
)
add_library(anvil::render ALIAS anvil_render)

target_include_directories(anvil_render PUBLIC include)
target_link_libraries(anvil_render
    PUBLIC
        anvil::core
    PRIVATE
        anvil_project_warnings
        Vulkan::Vulkan
)
set_target_properties(anvil_render PROPERTIES FOLDER "Engine")
anvil_enable_static_analysis(anvil_render)
