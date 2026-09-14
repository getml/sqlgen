project(sqlgen-mysql-tests)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(
    sqlgen-mysql-tests 
    ${SOURCES}
)
target_precompile_headers(sqlgen-mysql-tests PRIVATE [["sqlgen.hpp"]] <iostream> <string> <functional> <gtest/gtest.h>)

target_link_libraries(sqlgen-mysql-tests PRIVATE sqlgen_tests_crt)

add_custom_command(TARGET sqlgen-mysql-tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:sqlgen-mysql-tests> $<TARGET_RUNTIME_DLLS:sqlgen-mysql-tests>
  COMMAND_EXPAND_LISTS
)

if (WIN32 AND DEFINED VCPKG_TARGET_TRIPLET)
    file(GLOB VCPKG_RUNTIME_DLLS
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/*.dll"
    )

    if (VCPKG_RUNTIME_DLLS)
        add_custom_command(TARGET sqlgen-mysql-tests POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_RUNTIME_DLLS} $<TARGET_FILE_DIR:sqlgen-mysql-tests>
          COMMAND_EXPAND_LISTS
        )
    endif()
endif()

find_package(GTest)
gtest_discover_tests(sqlgen-mysql-tests)
