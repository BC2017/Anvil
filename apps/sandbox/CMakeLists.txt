add_executable(anvil_sandbox main.cpp)
target_link_libraries(anvil_sandbox PRIVATE anvil::platform anvil_project_warnings)
set_target_properties(anvil_sandbox PROPERTIES
    OUTPUT_NAME "AnvilSandbox"
    FOLDER "Applications"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:anvil_sandbox>"
)
anvil_enable_static_analysis(anvil_sandbox)

if(BUILD_TESTING)
    add_test(NAME anvil.sandbox.smoke COMMAND anvil_sandbox --smoke-test)
    set_tests_properties(anvil.sandbox.smoke PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")
endif()
