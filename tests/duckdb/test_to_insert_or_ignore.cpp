#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/duckdb.hpp>
#include <vector>

namespace test_to_insert_or_ignore {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t> id;
};

TEST(duckdb, test_to_insert_or_ignore) {
  const auto query =
      sqlgen::Insert<TestTable,
                     sqlgen::dynamic::Insert::ConflictPolicy::ignore>{};

  const auto expected =
      R"(INSERT OR IGNORE INTO "TestTable" BY NAME ( SELECT "field1" AS "field1", "id" AS "id" FROM sqlgen_appended_data);)";

  EXPECT_EQ(sqlgen::duckdb::to_sql(query), expected);
}

}  // namespace test_to_insert_or_ignore
