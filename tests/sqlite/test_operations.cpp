
#include <gtest/gtest.h>

#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_operations {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(sqlite, test_operations) {
  const auto people1 = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  using namespace sqlgen;

  struct Children {
    int id_plus_age;
    int age_times_2;
    int age_plus_2_minus_id;
    int age_mod_3;
  };

  const auto get_children =
      select_from<Person>(("id"_c + "age"_c) | as<"id_plus_age">,
                          ("age"_c * 2) | as<"age_times_2">,
                          ("age"_c % 3) | as<"age_mod_3">,
                          ("age"_c + 2 - "id"_c) | as<"age_plus_2_minus_id">) |
      where("age"_c < 18) | to<std::vector<Children>>;

  const auto children = sqlite::connect()
                            .and_then(write(std::ref(people1)))
                            .and_then(get_children)
                            .value();

  const std::string expected =
      R"([{"id_plus_age":11,"age_times_2":20,"age_plus_2_minus_id":11,"age_mod_3":1},{"id_plus_age":10,"age_times_2":16,"age_plus_2_minus_id":8,"age_mod_3":2},{"id_plus_age":3,"age_times_2":0,"age_plus_2_minus_id":-1,"age_mod_3":0}])";

  EXPECT_EQ(rfl::json::write(children), expected);
}

}  // namespace test_operations

