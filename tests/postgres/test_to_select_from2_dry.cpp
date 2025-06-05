#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>

namespace test_to_select_from2_dry {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::PrimaryKey<uint32_t> id;
  std::optional<std::string> nullable;
};

TEST(postgres, test_to_select_from2_dry) {
  using namespace sqlgen;

  const auto query =
      select_from<TestTable>("field1"_c | as<"field">,            //
                             avg("field2"_c) | as<"avg_field2">,  //
                             1 | as<"one">,                       //
                             "hello" | as<"hello">) |
      group_by("field1"_c);

  const auto expected =
      R"(SELECT "field1" AS "field", AVG("field2") AS "avg_field2", 1 AS "one", 'hello' AS "hello" FROM "TestTable" GROUP BY "field1";)";

  EXPECT_EQ(sqlgen::postgres::to_sql(query), expected);
}
}  // namespace test_to_select_from2_dry
