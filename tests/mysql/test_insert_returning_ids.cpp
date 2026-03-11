#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>

namespace test_insert_returning_ids {

struct Person {
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
  std::string first_name;
  int age;
};

TEST(mysql, test_insert_returning_ids) {
  const auto person = Person{.first_name = "Homer", .age = 45};

  auto ids = std::vector<uint32_t>{};

  const auto credentials = sqlgen::mysql::Credentials{.host = "localhost",
                                                      .user = "sqlgen",
                                                      .password = "password",
                                                      .dbname = "mysql"};

  using namespace sqlgen;

  const auto people_from_db =
      mysql::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(create_table<Person> | if_not_exists)
          .and_then(insert(std::ref(person), returning(ids)))
          .and_then(sqlgen::read<std::vector<Person>>)
          .value();

  ASSERT_EQ(ids.size(), 1u);
  ASSERT_EQ(people_from_db.size(), 1u);
  EXPECT_EQ(ids.at(0), people_from_db.at(0).id());
}

}  // namespace test_insert_returning_ids

#endif
