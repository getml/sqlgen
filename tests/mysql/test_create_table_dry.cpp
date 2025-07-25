#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>

namespace test_create_table_dry {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::PrimaryKey<uint32_t> id;
  std::optional<std::string> nullable;
};

TEST(mysql, test_create_table_dry) {
  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto query = create_table<TestTable> | if_not_exists;

  const auto expected =
      R"(CREATE TABLE IF NOT EXISTS `TestTable` (`field1` TEXT NOT NULL, `field2` INT NOT NULL, `id` INT UNSIGNED NOT NULL, `nullable` TEXT, PRIMARY KEY (`id`));)";

  EXPECT_EQ(sqlgen::mysql::to_sql(query), expected);
}
}  // namespace test_create_table_dry
