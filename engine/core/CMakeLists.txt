configure_file(
    include/anvil/core/version.hpp.in
    generated/anvil/core/version.hpp
    @ONLY
)

add_library(anvil_core STATIC
    src/assert.cpp
    src/log.cpp
)
add_library(anvil::core ALIAS anvil_core)

target_include_directories(anvil_core
    PUBLIC
        include
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_link_libraries(anvil_core
    PUBLIC anvil_project_options
    PRIVATE anvil_project_warnings
)
set_target_properties(anvil_core PROPERTIES FOLDER "Engine")
anvil_enable_static_analysis(anvil_core)
