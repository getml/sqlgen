#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>

namespace test_delete_from_dry {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::PrimaryKey<uint32_t> id;
  std::optional<std::string> nullable;
};

TEST(postgres, test_delete_from_dry) {
  using namespace sqlgen;

  const auto query = delete_from<TestTable> | where("field2"_c > 0);

  const auto expected = R"(DELETE FROM "TestTable" WHERE "field2" > 0;)";

  EXPECT_EQ(sqlgen::postgres::to_sql(query), expected);
}
}  // namespace test_delete_from_dry
