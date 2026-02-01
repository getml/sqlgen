#ifndef SQLGEN_TESTS_POSTGRES_TEST_HELPERS_HPP_
#define SQLGEN_TESTS_POSTGRES_TEST_HELPERS_HPP_

#include <cstdlib>
#include <sqlgen/postgres.hpp>

namespace sqlgen::postgres::test {

inline Credentials make_credentials() {
  const char* env_password = std::getenv("PGPASSWORD");
  return Credentials{.user = "postgres",
                     .password = env_password ? env_password : "password",
                     .host = "localhost",
                     .dbname = "postgres"};
}

}

#endif
