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

add_executable(anvil_gameplay_tests
    gameplay/system_graph_tests.cpp
)
target_link_libraries(anvil_gameplay_tests PRIVATE anvil::gameplay anvil_project_warnings)
set_target_properties(anvil_gameplay_tests PROPERTIES FOLDER "Tests")
anvil_enable_static_analysis(anvil_gameplay_tests)

add_test(NAME anvil.gameplay.system_graph COMMAND anvil_gameplay_tests)

if(TARGET anvil::editor)
    add_executable(anvil_editor_tests
        editor/command_history_tests.cpp
    )
    target_link_libraries(anvil_editor_tests PRIVATE anvil::editor anvil_project_warnings)
    set_target_properties(anvil_editor_tests PROPERTIES FOLDER "Tests")
    anvil_enable_static_analysis(anvil_editor_tests)

    add_test(NAME anvil.editor.history COMMAND anvil_editor_tests)
endif()

if(TARGET anvil::render)
    add_executable(anvil_render_data_tests
        render/render_data_tests.cpp
    )
    target_link_libraries(anvil_render_data_tests PRIVATE anvil::render anvil_project_warnings)
    set_target_properties(anvil_render_data_tests PROPERTIES FOLDER "Tests")
    anvil_enable_static_analysis(anvil_render_data_tests)
    add_test(NAME anvil.render.data COMMAND anvil_render_data_tests)

    add_executable(anvil_render_tests
        render/render_tests.cpp
    )
    target_link_libraries(anvil_render_tests PRIVATE anvil::render anvil_project_warnings)
    set_target_properties(anvil_render_tests PROPERTIES FOLDER "Tests")
    anvil_enable_static_analysis(anvil_render_tests)

    add_test(NAME anvil.render.bootstrap COMMAND anvil_render_tests)
    set_tests_properties(anvil.render.bootstrap PROPERTIES LABELS "requires_vulkan_icd")
endif()
