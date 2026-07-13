add_executable(anvil_core_tests
    unit/core_tests.cpp
)
target_link_libraries(anvil_core_tests PRIVATE anvil::core anvil_project_warnings)
set_target_properties(anvil_core_tests PROPERTIES FOLDER "Tests")
anvil_enable_static_analysis(anvil_core_tests)

add_test(NAME anvil.core COMMAND anvil_core_tests)

add_executable(anvil_assets_tests
    assets/assets_tests.cpp
)
target_link_libraries(anvil_assets_tests PRIVATE anvil::assets anvil_project_warnings)
set_target_properties(anvil_assets_tests PROPERTIES FOLDER "Tests")
anvil_enable_static_analysis(anvil_assets_tests)

add_test(NAME anvil.assets COMMAND anvil_assets_tests)

if(TARGET anvil::render)
    add_executable(anvil_render_tests
        render/render_tests.cpp
    )
    target_link_libraries(anvil_render_tests PRIVATE anvil::render anvil_project_warnings)
    set_target_properties(anvil_render_tests PROPERTIES FOLDER "Tests")
    anvil_enable_static_analysis(anvil_render_tests)

    add_test(NAME anvil.render.bootstrap COMMAND anvil_render_tests)
    set_tests_properties(anvil.render.bootstrap PROPERTIES LABELS "requires_vulkan_icd")
endif()
