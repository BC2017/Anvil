function(anvil_enable_static_analysis target)
    if(NOT ANVIL_ENABLE_CLANG_TIDY)
        return()
    endif()

    find_program(ANVIL_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
    set_property(
        TARGET ${target}
        PROPERTY CXX_CLANG_TIDY
            "${ANVIL_CLANG_TIDY_EXECUTABLE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy"
    )
endfunction()
