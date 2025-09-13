#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>

namespace test_json {

struct Person {
  std::string first_name;
  std::string last_name;
  int age;
  sqlgen::JSON<std::optional<std::vector<Person>>> children;
};

TEST(mysql, test_json) {
  const auto children = std::vector<Person>(
      {Person{.first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{.first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  const auto homer = Person{.first_name = "Homer",
                            .last_name = "Simpson",
                            .age = 45,
                            .children = children};

  const auto credentials = sqlgen::mysql::Credentials{.host = "localhost",
                                                      .user = "sqlgen",
                                                      .password = "password",
                                                      .dbname = "mysql"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto people2 = sqlgen::mysql::connect(credentials)
                           .and_then(drop<Person> | if_exists)
                           .and_then(write(std::ref(homer)))
                           .and_then(sqlgen::read<std::vector<Person>>)
                           .value();

  const std::string expected =
      R"([{"first_name":"Homer","last_name":"Simpson","age":45,"children":[{"first_name":"Bart","last_name":"Simpson","age":10},{"first_name":"Lisa","last_name":"Simpson","age":8},{"first_name":"Maggie","last_name":"Simpson","age":0}]}])";

  EXPECT_EQ(rfl::json::write(people2), expected);
}

}  // namespace test_json

#endif
