function(anvil_set_project_warnings target)
    if(MSVC)
        set(warnings /W4 /permissive- /Zc:preprocessor /Zc:__cplusplus)
        if(ANVIL_WARNINGS_AS_ERRORS)
            list(APPEND warnings /WX)
        endif()
    else()
        set(warnings
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
        )
        if(ANVIL_WARNINGS_AS_ERRORS)
            list(APPEND warnings -Werror)
        endif()
    endif()

    target_compile_options(${target} INTERFACE ${warnings})
endfunction()
