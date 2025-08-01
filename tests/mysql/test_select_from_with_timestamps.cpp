#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <chrono>
#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>

namespace test_select_from_with_timestamps {

struct Person {
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
  std::string first_name;
  std::string last_name;
  sqlgen::Date birthday;
};

TEST(mysql, test_select_from_with_timestamps) {
  const auto people1 =
      std::vector<Person>({Person{.first_name = "Homer",
                                  .last_name = "Simpson",
                                  .birthday = sqlgen::Date("1970-01-01")},
                           Person{.first_name = "Bart",
                                  .last_name = "Simpson",
                                  .birthday = sqlgen::Date("2000-01-01")},
                           Person{.first_name = "Lisa",
                                  .last_name = "Simpson",
                                  .birthday = sqlgen::Date("2002-01-01")},
                           Person{.first_name = "Maggie",
                                  .last_name = "Simpson",
                                  .birthday = sqlgen::Date("2010-01-01")}});

  const auto credentials = sqlgen::mysql::Credentials{.host = "localhost",
                                                      .user = "sqlgen",
                                                      .password = "password",
                                                      .dbname = "mysql"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  struct Birthday {
    Date birthday;
    Date birthday_recreated;
    double age_in_days;
    int hour;
    int minute;
    int second;
    int weekday;
  };

  const auto get_birthdays =
      select_from<Person>(
          ("birthday"_c + std::chrono::weeks(2) + std::chrono::days(-4)) |
              as<"birthday">,
          ((cast<Date>(concat(cast<std::string>(year("birthday"_c)), "-",
                              cast<std::string>(month("birthday"_c)), "-",
                              cast<std::string>(day("birthday"_c))))) +
           std::chrono::days(10)) |
              as<"birthday_recreated">,
          days_between("birthday"_c, Date("2011-01-01")) | as<"age_in_days">,
          hour("birthday"_c) | as<"hour">, minute("birthday"_c) | as<"minute">,
          second("birthday"_c) | as<"second">,
          weekday("birthday"_c) | as<"weekday">) |
      order_by("id"_c) | to<std::vector<Birthday>>;

  const auto birthdays = mysql::connect(credentials)
                             .and_then(drop<Person> | if_exists)
                             .and_then(write(std::ref(people1)))
                             .and_then(get_birthdays)
                             .value();

  const std::string expected_query =
      R"(SELECT date_add(date_add(`birthday`, INTERVAL 2 week), INTERVAL -4 day) AS `birthday`, date_add(cast(concat(cast(extract(YEAR from `birthday`) as CHAR), '-', cast(extract(MONTH from `birthday`) as CHAR), '-', cast(extract(DAY from `birthday`) as CHAR)) as DATE), INTERVAL 10 day) AS `birthday_recreated`, datediff('2011-01-01', `birthday`) AS `age_in_days`, extract(HOUR from `birthday`) AS `hour`, extract(MINUTE from `birthday`) AS `minute`, extract(SECOND from `birthday`) AS `second`, dayofweek(`birthday`) AS `weekday` FROM `Person` ORDER BY `id`)";

  const std::string expected =
      R"([{"birthday":"1970-01-11","birthday_recreated":"1970-01-11","age_in_days":14975.0,"hour":0,"minute":0,"second":0,"weekday":5},{"birthday":"2000-01-11","birthday_recreated":"2000-01-11","age_in_days":4018.0,"hour":0,"minute":0,"second":0,"weekday":7},{"birthday":"2002-01-11","birthday_recreated":"2002-01-11","age_in_days":3287.0,"hour":0,"minute":0,"second":0,"weekday":3},{"birthday":"2010-01-11","birthday_recreated":"2010-01-11","age_in_days":365.0,"hour":0,"minute":0,"second":0,"weekday":6}])";

  EXPECT_EQ(mysql::to_sql(get_birthdays), expected_query);
  EXPECT_EQ(rfl::json::write(birthdays), expected);
}

}  // namespace test_select_from_with_timestamps

#endif
