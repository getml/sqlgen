#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>

namespace test_insert_dry {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::PrimaryKey<uint32_t> id;
  std::optional<std::string> nullable;
};

TEST(postgres, test_insert_dry) {
  const auto query = sqlgen::Insert<TestTable>{};

  const auto expected =
      "COPY \"public\".\"TestTable\"(\"field1\", \"field2\", \"id\", "
      "\"nullable\") FROM STDIN WITH DELIMITER '\t' NULL '\e' QUOTE '\a';";

  EXPECT_EQ(sqlgen::postgres::to_sql(query), expected);
}
}  // namespace test_insert_dry
