#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>

namespace test_insert_or_ignore_dry {

struct TestTable {
  std::string field1;
  sqlgen::PrimaryKey<uint32_t> id;
};

TEST(mysql, test_insert_or_ignore_dry) {
  const auto query =
      sqlgen::Insert<TestTable,
                     sqlgen::dynamic::Insert::ConflictPolicy::ignore>{};

  const auto expected =
      R"(INSERT IGNORE INTO `TestTable` (`field1`, `id`) VALUES (?, ?);)";

  EXPECT_EQ(sqlgen::mysql::to_sql(query), expected);
}

}  // namespace test_insert_or_ignore_dry
