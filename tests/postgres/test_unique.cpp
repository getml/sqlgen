#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_unique {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  sqlgen::Unique<std::string> first_name;
  std::string last_name;
  double age;
};

TEST(postgres, test_unique) {
  const auto people = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{
           .id = 1, .first_name = "Marge", .last_name = "Simpson", .age = 40},
       Person{.id = 2, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 3, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 4, .first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  postgres::connect(credentials)
      .and_then(drop<Person> | if_exists)
      .and_then(create_table<Person>)
      .and_then(insert(std::ref(people)))
      .value();

  const std::string expected_query =
      R"(CREATE TABLE IF NOT EXISTS "Person" ("id" INTEGER NOT NULL, "first_name" TEXT NOT NULL UNIQUE, "last_name" TEXT NOT NULL, "age" NUMERIC NOT NULL, PRIMARY KEY ("id"));)";

  EXPECT_EQ(postgres::to_sql(create_table<Person>), expected_query);
}

}  // namespace test_unique

#endif
