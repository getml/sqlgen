#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>

namespace test_timestamp {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S"> ts;
};

TEST(postgres, test_timestamp) {
  const auto people1 =
      std::vector<Person>({Person{.id = 0,
                                  .first_name = "Homer",
                                  .last_name = "Simpson",
                                  .ts = "2000-01-01 01:00:00"}});

  using namespace sqlgen;

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  const auto conn =
      sqlgen::postgres::connect(credentials).and_then(drop<Person> | if_exists);

  const auto people2 = sqlgen::write(conn, people1)
                           .and_then(sqlgen::read<std::vector<Person>>)
                           .value();

  const auto json1 = rfl::json::write(people1);
  const auto json2 = rfl::json::write(people2);

  EXPECT_EQ(json1, json2);
}

}  // namespace test_timestamp

#endif
