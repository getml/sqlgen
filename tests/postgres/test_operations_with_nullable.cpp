#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_operations_with_nullable {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  std::optional<int> age;
};

TEST(postgres, test_operations_with_nullable) {
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

  struct Children {
    std::optional<int> id_plus_age;
    std::optional<int> age_times_2;
    std::optional<int> id_plus_2_minus_age;
  };

  const auto get_children =
      select_from<Person>(("id"_c + "age"_c) | as<"id_plus_age">,
                          ("age"_c * 2) | as<"age_times_2">,
                          ("id"_c + 2 - "age"_c) | as<"id_plus_2_minus_age">) |
      where("age"_c < 18) | to<std::vector<Children>>;

  const auto children = postgres::connect(credentials)
                            .and_then(drop<Person> | if_exists)
                            .and_then(write(std::ref(people1)))
                            .and_then(get_children)
                            .value();

  const std::string expected =
      R"([{"id_plus_age":11,"age_times_2":20,"id_plus_2_minus_age":-7},{"id_plus_age":10,"age_times_2":16,"id_plus_2_minus_age":-4},{"id_plus_age":3,"age_times_2":0,"id_plus_2_minus_age":5}])";

  EXPECT_EQ(rfl::json::write(children), expected);
}

}  // namespace test_operations_with_nullable

#endif
