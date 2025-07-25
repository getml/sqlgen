#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>

namespace test_timestamp_with_tz_dry {

struct TestTable {
  std::string field1;
  int32_t field2;
  sqlgen::PrimaryKey<uint32_t> id;
  sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S%z"> ts;
};

TEST(mysql, test_timestamp_with_tz_dry) {
  const auto query = sqlgen::CreateTable<TestTable>{};

  const auto expected =
      R"(CREATE TABLE IF NOT EXISTS `TestTable` (`field1` TEXT NOT NULL, `field2` INT NOT NULL, `id` INT UNSIGNED NOT NULL, `ts` TIMESTAMP NOT NULL, PRIMARY KEY (`id`));)";

  EXPECT_EQ(sqlgen::mysql::to_sql(query), expected);
}
}  // namespace test_timestamp_with_tz_dry
