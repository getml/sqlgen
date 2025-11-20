#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_union {

struct User1 {
  std::string name;
  int age;
};

struct User2 {
  std::string name;
  int age;
};

struct User3 {
  std::string name;
  int age;
};

TEST(sqlite, test_union) {
  // Connect to SQLite database
  const auto conn = sqlgen::sqlite::connect("test.db");

  // Create and insert a user
  const auto user1 = User1{.name = "John", .age = 30};
  sqlgen::write(conn, user1);

  const auto user2 = User2{.name = "Jane", .age = 25};
  sqlgen::write(conn, user2);

  const auto user3 = User3{.name = "Joe", .age = 40};
  sqlgen::write(conn, user3);

  const auto s1 = sqlgen::select_from<User1>();
  const auto s2 = sqlgen::select_from<User2>();
  const auto s3 = sqlgen::select_from<User3>();

  const auto result = sqlgen::union_(s1, s2, s3)(conn);

  const auto users = result.value();

  EXPECT_EQ(users.size(), 3);
  EXPECT_EQ(users.at(0).name, "John");
  EXPECT_EQ(users.at(0).age, 30);
  EXPECT_EQ(users.at(1).name, "Jane");
  EXPECT_EQ(users.at(1).age, 25);
  EXPECT_EQ(users.at(2).name, "Joe");
  EXPECT_EQ(users.at(2).age, 40);
}

}  // namespace test_union
