#include <gtest/gtest.h>

#include <iostream>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_write_and_read_dry {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(postgres, test_write_and_read_dry) {
  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto query1 =
      postgres::to_sql(sqlgen::read<std::vector<Person>> | where("id"_c == 1));
  const auto query2 =
      postgres::to_sql(sqlgen::read<Person> | where("id"_c == 1));

  std::cout << query1 << std::endl;
  std::cout << query2 << std::endl;
}

}  // namespace test_write_and_read_dry

