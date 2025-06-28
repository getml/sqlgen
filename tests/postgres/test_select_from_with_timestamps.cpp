#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <chrono>
#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_range_select_from_with_timestamps {

struct Person {
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
  std::string first_name;
  std::string last_name;
  sqlgen::Timestamp<"%Y-%m-%d"> birthday;
};

TEST(postgres, test_range_select_from_with_timestamps) {
  const auto people1 = std::vector<Person>(
      {Person{.first_name = "Homer",
              .last_name = "Simpson",
              .birthday = sqlgen::Timestamp<"%Y-%m-%d">("1970-01-01")},
       Person{.first_name = "Bart",
              .last_name = "Simpson",
              .birthday = sqlgen::Timestamp<"%Y-%m-%d">("2000-01-01")},
       Person{.first_name = "Lisa",
              .last_name = "Simpson",
              .birthday = sqlgen::Timestamp<"%Y-%m-%d">("2002-01-01")},
       Person{.first_name = "Maggie",
              .last_name = "Simpson",
              .birthday = sqlgen::Timestamp<"%Y-%m-%d">("2010-01-01")}});

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  using namespace sqlgen;

  struct Birthday {
    sqlgen::Timestamp<"%Y-%m-%d"> birthday;
  };

  const auto birthdays =
      postgres::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(write(std::ref(people1)))
          .and_then(
              select_from<Person>(
                  ("birthday"_c + std::chrono::days(10)).as<"birthday">()) |
              order_by("id"_c) | to<std::vector<Birthday>>)
          .value();

  const std::string expected =
      R"([{"birthday":"1970-01-11"},{"birthday":"2000-01-11"},{"birthday":"2002-01-11"},{"birthday":"2010-01-11"}])";

  EXPECT_EQ(rfl::json::write(birthdays), expected);
}

}  // namespace test_range_select_from_with_timestamps

#endif
