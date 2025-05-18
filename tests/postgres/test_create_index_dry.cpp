#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>

namespace test_create_index_dry {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::PrimaryKey<uint32_t> id;
  std::optional<std::string> nullable;
};

TEST(postgres, test_create_index_dry) {
  using namespace sqlgen;

  const auto query =
      create_index<"test_table_ix", TestTable>("field1"_c, "field2"_c) |
      if_not_exists;

  const auto expected =
      R"(CREATE INDEX IF NOT EXISTS "test_table_ix" ON "TestTable"("field1", "field2");)";

  EXPECT_EQ(sqlgen::postgres::to_sql(query), expected);
}
}  // namespace test_create_index_dry
