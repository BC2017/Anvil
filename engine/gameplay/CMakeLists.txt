add_library(anvil_gameplay STATIC
    src/system_graph.cpp
)
add_library(anvil::gameplay ALIAS anvil_gameplay)

target_include_directories(anvil_gameplay PUBLIC include)
target_link_libraries(anvil_gameplay
    PUBLIC
        anvil::core
    PRIVATE
        anvil_project_warnings
)
set_target_properties(anvil_gameplay PROPERTIES FOLDER "Engine")
anvil_enable_static_analysis(anvil_gameplay)
