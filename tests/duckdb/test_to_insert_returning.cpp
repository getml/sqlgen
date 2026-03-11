#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/duckdb.hpp>
#include <vector>

namespace test_to_insert_returning {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
};

TEST(duckdb, test_to_insert_returning) {
  const auto query =
      sqlgen::Insert<TestTable, sqlgen::dynamic::Insert::ConflictPolicy::none,
                     std::vector<uint32_t>>{};

  const auto expected =
      R"(INSERT INTO "TestTable" BY NAME ( SELECT "field1" AS "field1" FROM sqlgen_appended_data) RETURNING "id";)";

  EXPECT_EQ(sqlgen::duckdb::to_sql(query), expected);
}

}  // namespace test_to_insert_returning
