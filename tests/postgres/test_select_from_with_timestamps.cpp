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
    Date birthday;
    time_t birthday_unixepoch;
    double age_in_days;
  };

  const auto get_birthdays =
      select_from<Person>(
          ("birthday"_c + std::chrono::days(10)) | as<"birthday">,
          days_between("birthday"_c, Date("2011-01-01")) | as<"age_in_days">,
          unixepoch("birthday"_c + std::chrono::days(10)) |
              as<"birthday_unixepoch">) |
      order_by("id"_c) | to<std::vector<Birthday>>;

  const auto birthdays = postgres::connect(credentials)
                             .and_then(drop<Person> | if_exists)
                             .and_then(write(std::ref(people1)))
                             .and_then(get_birthdays)
                             .value();

  const std::string expected =
      R"([{"birthday":"1970-01-11","birthday_unixepoch":864000,"age_in_days":14975.0},{"birthday":"2000-01-11","birthday_unixepoch":947548800,"age_in_days":4018.0},{"birthday":"2002-01-11","birthday_unixepoch":1010707200,"age_in_days":3287.0},{"birthday":"2010-01-11","birthday_unixepoch":1263168000,"age_in_days":365.0}])";

  EXPECT_EQ(rfl::json::write(birthdays), expected);
}

}  // namespace test_range_select_from_with_timestamps

#endif
