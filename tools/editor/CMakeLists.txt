add_library(anvil_editor STATIC
    src/command_history.cpp
)
add_library(anvil::editor ALIAS anvil_editor)

target_include_directories(anvil_editor PUBLIC include)
target_link_libraries(anvil_editor
    PUBLIC
        anvil::core
    PRIVATE
        anvil_project_warnings
)
set_target_properties(anvil_editor PROPERTIES FOLDER "Tools")
anvil_enable_static_analysis(anvil_editor)
