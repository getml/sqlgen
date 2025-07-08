#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_join {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  double age;
};

TEST(postgres, test_join) {
  static_assert(std::ranges::input_range<sqlgen::Range<Person>>,
                "Must be an input range.");

  const auto people1 = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  using namespace sqlgen;

  const auto get_people =
      select_from<Person, "t1">(
          "id"_t1 | as<"id">, "first_name"_t1 | as<"first_name">,
          "last_name"_t2 | as<"last_name">, "age"_t2 | as<"age">) |
      left_join<Person, "t2">("id"_t1 == "id"_t2) | to<std::vector<Person>>;

  std::cout << postgres::to_sql(get_people) << std::endl;

  const auto people = postgres::connect(credentials)
                          .and_then(drop<Person> | if_exists)
                          .and_then(write(std::ref(people1)))
                          .and_then(get_people)
                          .value();

  std::cout << rfl::json::write(people) << std::endl;
}

}  // namespace test_join

#endif
