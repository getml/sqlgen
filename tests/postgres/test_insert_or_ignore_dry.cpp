#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_insert_or_ignore_dry {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t> id;
};

TEST(postgres, test_insert_or_ignore_dry) {
  const auto query =
      sqlgen::Insert<TestTable,
                     sqlgen::dynamic::Insert::ConflictPolicy::ignore>{};

  const auto expected =
      R"(INSERT INTO "TestTable" ("field1", "id") VALUES ($1, $2) ON CONFLICT DO NOTHING;)";

  EXPECT_EQ(sqlgen::postgres::to_sql(query), expected);
}

}  // namespace test_insert_or_ignore_dry
