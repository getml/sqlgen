project(sqlgen-mysql-tests)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(
    sqlgen-mysql-tests 
    ${SOURCES}
)
target_precompile_headers(sqlgen-mysql-tests PRIVATE [["sqlgen.hpp"]] <iostream> <string> <functional> <gtest/gtest.h>)


target_link_libraries(
    sqlgen-mysql-tests
    PRIVATE 
    "${SQLGEN_GTEST_LIB}"
)

find_package(GTest)
gtest_discover_tests(sqlgen-mysql-tests)
