
#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>

namespace test_varchar {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  sqlgen::Varchar<6> first_name;
  sqlgen::Varchar<7> last_name;
  int age;
};

TEST(mysql, test_varchar) {
  const auto people1 = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  const auto credentials =
      sqlgen::mysql::Credentials{.host = "localhost",
                                 .user = "sqlgen",
                                 .password = "password",
                                 .dbname = "mysql",
                                 .unix_socket = "/var/run/mysqld/mysqld.sock"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto conn =
      sqlgen::mysql::connect(credentials).and_then(drop<Person> | if_exists);

  sqlgen::write(conn, people1).value();

  const auto people2 = sqlgen::read<std::vector<Person>>(conn).value();

  const auto json1 = rfl::json::write(people1);
  const auto json2 = rfl::json::write(people2);

  EXPECT_EQ(json1, json2);
}

}  // namespace test_varchar

#endif
