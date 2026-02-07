#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_to_insert_or_ignore {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t> id;
};

TEST(sqlite, test_to_insert_or_ignore) {
  const auto query =
      sqlgen::Insert<TestTable,
                     sqlgen::dynamic::Insert::ConflictPolicy::ignore>{};

  const auto expected =
      R"(INSERT OR IGNORE INTO "TestTable" ("field1", "id") VALUES (?, ?);)";

  EXPECT_EQ(sqlgen::sqlite::to_sql(query), expected);
}

}  // namespace test_to_insert_or_ignore
