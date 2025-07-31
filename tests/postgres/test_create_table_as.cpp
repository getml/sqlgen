#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_create_table_as {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

struct Name {
  std::string first_name;
  std::string last_name;
};

TEST(postgres, test_create_table_as) {
  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto people = sqlgen::postgres::connect(credentials)
                          .and_then(drop<Person> | if_exists)
                          .and_then(create_table_as<Name>(select_from<Person>(
                                        "first_name"_c, "last_name"_c)) |
                                    if_not_exists)
                          .value();

  const std::string expected = R"([])";

  EXPECT_EQ(rfl::json::write(people), expected);
}

}  // namespace test_create_table_as

#endif
