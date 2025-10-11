project(sqlgen-duckdb-tests)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(
    sqlgen-duckdb-tests 
    ${SOURCES}
)
target_precompile_headers(sqlgen-duckdb-tests PRIVATE [["sqlgen.hpp"]] <iostream> <string> <functional> <gtest/gtest.h>)


target_link_libraries(
    sqlgen-duckdb-tests
    PRIVATE 
    "${SQLGEN_GTEST_LIB}"
)

find_package(GTest)
gtest_discover_tests(sqlgen-duckdb-tests)
