
#include <gtest/gtest.h>

#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_range_select_from {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(sqlite, test_range_select_from) {
  const auto people1 = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  using namespace sqlgen;

  const auto people2 =
      sqlgen::sqlite::connect()
          .and_then(sqlgen::write(std::ref(people1)))
          .and_then(select_from<Person>("first_name"_c) | order_by("id"_c))
          .value();

  using namespace std::ranges::views;

  const auto first_names =
      internal::collect::vector(people2 | transform([](const auto& _r) {
                                  return rfl::get<"first_name">(_r.value());
                                }));

  EXPECT_EQ(first_names.at(0), "Homer");
  EXPECT_EQ(first_names.at(1), "Bart");
  EXPECT_EQ(first_names.at(2), "Lisa");
  EXPECT_EQ(first_names.at(3), "Maggie");
}

}  // namespace test_range_select_from

