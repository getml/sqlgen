#ifndef SQLGEN_TESTS_MYSQL_TEST_HELPERS_HPP_
#define SQLGEN_TESTS_MYSQL_TEST_HELPERS_HPP_

#include <cstdlib>
#include <sqlgen/mysql.hpp>

namespace sqlgen::mysql::test {

inline Credentials make_credentials() {
  const char* env_password = std::getenv("SQLGEN_TEST_PASSWORD");
  return Credentials{.host = "localhost",
                     .user = "sqlgen",
                     .password = env_password ? env_password : "password",
                     .dbname = "mysql"};
}

}

#endif
