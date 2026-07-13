foreach(required_variable
        ANVIL_BUILD_DIR
        ANVIL_CONFIG
        ANVIL_EXECUTABLE_NAME
        ANVIL_EXPECTED_VERSION
        ANVIL_TEST_PREFIX)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "Missing required variable: ${required_variable}")
    endif()
endforeach()

set(installed_executable "${ANVIL_TEST_PREFIX}/bin/${ANVIL_EXECUTABLE_NAME}")
set(installed_manifest "${ANVIL_TEST_PREFIX}/anvil-runtime.json")
file(REMOVE "${installed_executable}" "${installed_manifest}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${ANVIL_BUILD_DIR}"
            --config "${ANVIL_CONFIG}" --prefix "${ANVIL_TEST_PREFIX}"
    RESULT_VARIABLE install_result
    OUTPUT_VARIABLE install_output
    ERROR_VARIABLE install_error
)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Install failed:\n${install_output}\n${install_error}")
endif()

if(NOT EXISTS "${installed_executable}")
    message(FATAL_ERROR "Installed runtime is missing: ${installed_executable}")
endif()
if(NOT EXISTS "${installed_manifest}")
    message(FATAL_ERROR "Installed runtime manifest is missing: ${installed_manifest}")
endif()

file(READ "${installed_manifest}" manifest_content)
string(JSON manifest_schema GET "${manifest_content}" schema_version)
string(JSON manifest_version GET "${manifest_content}" engine_version)
string(JSON manifest_configuration GET "${manifest_content}" build_configuration)
if(NOT manifest_schema EQUAL 1)
    message(FATAL_ERROR "Unsupported runtime manifest schema: ${manifest_schema}")
endif()
if(NOT manifest_version STREQUAL "${ANVIL_EXPECTED_VERSION}")
    message(FATAL_ERROR
        "Runtime manifest version '${manifest_version}' did not match '${ANVIL_EXPECTED_VERSION}'"
    )
endif()
if(NOT manifest_configuration STREQUAL "${ANVIL_CONFIG}")
    message(FATAL_ERROR
        "Runtime manifest configuration '${manifest_configuration}' did not match '${ANVIL_CONFIG}'"
    )
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env SDL_VIDEODRIVER=dummy
            "${installed_executable}" --smoke-test
    RESULT_VARIABLE runtime_result
    OUTPUT_VARIABLE runtime_output
    ERROR_VARIABLE runtime_error
)
if(NOT runtime_result EQUAL 0)
    message(FATAL_ERROR "Installed runtime failed:\n${runtime_output}\n${runtime_error}")
endif()
