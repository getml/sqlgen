#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_to_insert_returning {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
};

TEST(sqlite, test_to_insert_returning) {
  // Use a concrete IDs type (e.g., std::vector<uint32_t>) instead of bool
  const auto query =
      sqlgen::Insert<TestTable, sqlgen::dynamic::Insert::ConflictPolicy::none,
                     std::vector<uint32_t>>{};

  const auto expected =
      R"(INSERT INTO "TestTable" ("field1") VALUES (?) RETURNING "id";)";

  EXPECT_EQ(sqlgen::sqlite::to_sql(query), expected);
}

}  // namespace test_to_insert_returning
