include(GNUInstallDirs)

set(ANVIL_RUNTIME_MANIFEST
    "${CMAKE_CURRENT_BINARY_DIR}/generated/$<CONFIG>/anvil-runtime.json"
)
string(CONFIGURE [=[{
  "schema_version": 1,
  "engine_version": "@PROJECT_VERSION@",
  "build_configuration": "$<CONFIG>",
  "system": "@CMAKE_SYSTEM_NAME@",
  "architecture": "@CMAKE_SYSTEM_PROCESSOR@"
}
]=] ANVIL_RUNTIME_MANIFEST_CONTENT @ONLY)
file(GENERATE
    OUTPUT "${ANVIL_RUNTIME_MANIFEST}"
    CONTENT "${ANVIL_RUNTIME_MANIFEST_CONTENT}"
)

install(
    TARGETS anvil_sandbox
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Runtime
)
install(
    FILES
        "${ANVIL_RUNTIME_MANIFEST}"
        "${CMAKE_SOURCE_DIR}/README.md"
        "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION "."
    COMPONENT Runtime
)

if(BUILD_TESTING)
    add_test(
        NAME anvil.package.install
        COMMAND "${CMAKE_COMMAND}"
            "-DANVIL_BUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DANVIL_CONFIG=$<CONFIG>"
            "-DANVIL_EXECUTABLE_NAME=$<TARGET_FILE_NAME:anvil_sandbox>"
            "-DANVIL_EXPECTED_VERSION=${PROJECT_VERSION}"
            "-DANVIL_TEST_PREFIX=${CMAKE_BINARY_DIR}/package-smoke/$<CONFIG>"
            -P "${CMAKE_SOURCE_DIR}/cmake/VerifyInstall.cmake"
    )
    set_tests_properties(anvil.package.install PROPERTIES LABELS "packaging")
endif()

set(CPACK_PACKAGE_NAME "Anvil")
set(CPACK_PACKAGE_VENDOR "Anvil Engine")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Anvil cross-platform 3D game engine runtime")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME
    "Anvil-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
)
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_MONOLITHIC_INSTALL ON)

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
