#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_insert_returning_dry {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
};

TEST(postgres, test_insert_returning_dry) {
  const auto query =
      sqlgen::Insert<TestTable, sqlgen::dynamic::Insert::ConflictPolicy::none,
                     std::vector<uint32_t>>{};

  const auto expected =
      R"(INSERT INTO "TestTable" ("field1") VALUES ($1) RETURNING "id";)";

  EXPECT_EQ(sqlgen::postgres::to_sql(query), expected);
}

}  // namespace test_insert_returning_dry
