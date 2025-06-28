#include <gtest/gtest.h>

#include <chrono>
#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_range_select_from_with_timestamps {

struct Person {
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
  std::string first_name;
  std::string last_name;
  sqlgen::Timestamp<"%Y-%m-%d"> birthday;
};

TEST(sqlite, test_range_select_from_with_timestamps) {
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

  using namespace sqlgen;

  struct Birthday {
    sqlgen::Date birthday;
  };

  const auto query =
      select_from<Person>(
          ("birthday"_c + std::chrono::days(10)).as<"birthday">()) |
      order_by("id"_c) | to<std::vector<Birthday>>;

  const auto birthdays = sqlite::connect()
                             .and_then(write(std::ref(people1)))
                             .and_then(query)
                             .value();

  const std::string expected_query =
      R"(SELECT datetime("birthday", '+10 days') AS "birthday" FROM "Person" ORDER BY "id";)";
  const std::string expected =
      R"([{"birthday":"1970-01-11"},{"birthday":"2000-01-11"},{"birthday":"2002-01-11"},{"birthday":"2010-01-11"}])";

  EXPECT_EQ(sqlite::to_sql(query), expected_query);
  EXPECT_EQ(rfl::json::write(birthdays), expected);
}

}  // namespace test_range_select_from_with_timestamps

