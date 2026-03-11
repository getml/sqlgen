#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_to_insert_or_replace_tag {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::Unique<std::string> field3;
  sqlgen::PrimaryKey<uint32_t> id;
  std::optional<std::string> nullable;
};

TEST(sqlite, test_to_insert_or_replace_tag) {
  const auto query =
      sqlgen::Insert<TestTable,
                     sqlgen::dynamic::Insert::ConflictPolicy::replace>{};

  const auto expected =
      R"(INSERT INTO "TestTable" ("field1", "field2", "field3", "id", "nullable") VALUES (?, ?, ?, ?, ?) ON CONFLICT (field3, id) DO UPDATE SET field1=excluded.field1, field2=excluded.field2, field3=excluded.field3, id=excluded.id, nullable=excluded.nullable;)";

  EXPECT_EQ(sqlgen::sqlite::to_sql(query), expected);
}

}  // namespace test_to_insert_or_replace_tag
