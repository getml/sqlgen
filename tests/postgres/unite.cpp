#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

#include "sqlgen/postgres/to_sql.hpp"

namespace test_unite {

struct User1 {
  std::string name;
  int age;
};

struct User2 {
  std::string name;
  int age;
};

struct User3 {
  int age;
  std::string name;
};

struct Login {
  int id;
  std::string username;
};

TEST(postgres, test_union) {
  using namespace sqlgen::literals;

  // Connect to postgres database
  const auto conn = sqlgen::postgres::connect();

  // Create and insert a user
  const auto user1 = User1{.name = "John", .age = 30};
  sqlgen::write(conn, user1);

  const auto user2 = User2{.name = "Jane", .age = 25};
  sqlgen::write(conn, user2);

  const auto user3 = User3{.age = 40, .name = "Joe"};
  sqlgen::write(conn, user3);

  const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
  const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);
  const auto s3 = sqlgen::select_from<User3>("name"_c, "age"_c);

  const auto result = sqlgen::unite<std::vector<User1>>(s1, s2, s3)(conn);

  const auto users = result.value();

  const auto query =
      sqlgen::postgres::to_sql(sqlgen::unite<std::vector<User1>>(s1, s2, s3));

  EXPECT_EQ(
      query,
      R"(SELECT "name", "age" FROM (SELECT "name", "age" FROM "User1") UNION SELECT "name", "age" FROM (SELECT "name", "age" FROM "User2") UNION SELECT "name", "age" FROM (SELECT "name", "age" FROM "User3");)");

  EXPECT_EQ(users.size(), 3);
  EXPECT_EQ(users.at(0).name, "Jane");
  EXPECT_EQ(users.at(0).age, 25);
  EXPECT_EQ(users.at(1).name, "Joe");
  EXPECT_EQ(users.at(1).age, 40);
  EXPECT_EQ(users.at(2).name, "John");
  EXPECT_EQ(users.at(2).age, 30);
}

TEST(postgres, test_union_all) {
  using namespace sqlgen::literals;

  // Connect to postgres database
  const auto conn = sqlgen::postgres::connect();

  // Create and insert a user
  const auto user1 = User1{.name = "John", .age = 30};
  sqlgen::write(conn, user1);

  const auto user2 = User2{.name = "Jane", .age = 25};
  sqlgen::write(conn, user2);

  const auto user3 = User3{.age = 30, .name = "John"};
  sqlgen::write(conn, user3);

  const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
  const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);
  const auto s3 = sqlgen::select_from<User3>("name"_c, "age"_c);

  const auto result = sqlgen::unite_all<std::vector<User1>>(s1, s2, s3)(conn);

  const auto users = result.value();

  const auto query =
      sqlgen::postgres::to_sql(sqlgen::unite_all<std::vector<User1>>(s1, s2, s3));

  EXPECT_EQ(
      query,
      R"(SELECT "name", "age" FROM (SELECT "name", "age" FROM "User1") UNION ALL SELECT "name", "age" FROM (SELECT "name", "age" FROM "User2") UNION ALL SELECT "name", "age" FROM (SELECT "name", "age" FROM "User3");)");

  EXPECT_EQ(users.size(), 3);
  EXPECT_EQ(users.at(0).name, "John");
  EXPECT_EQ(users.at(0).age, 30);
  EXPECT_EQ(users.at(1).name, "Jane");
  EXPECT_EQ(users.at(1).age, 25);
  EXPECT_EQ(users.at(2).name, "John");
  EXPECT_EQ(users.at(2).age, 30);
}

TEST(postgres, test_union_in_select) {
  using namespace sqlgen::literals;

  // Connect to postgres database
  const auto conn = sqlgen::postgres::connect();

  // Create and insert a user
  const auto user1 = User1{.name = "John", .age = 30};
  sqlgen::write(conn, user1);

  const auto user2 = User2{.name = "Jane", .age = 25};
  sqlgen::write(conn, user2);

  const auto user3 = User3{.age = 40, .name = "Joe"};
  sqlgen::write(conn, user3);

  const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
  const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);
  const auto s3 = sqlgen::select_from<User3>("name"_c, "age"_c);

  const auto united = sqlgen::unite<std::vector<User1>>(s1, s2, s3);

  const auto sel = sqlgen::select_from(united.as("u"), "name"_c, "age"_c);

  const auto result = sel(conn);

  const auto users = result.value();

  const auto query = sqlgen::postgres::to_sql(sel);

  EXPECT_EQ(
      query,
      R"(SELECT "name", "age" FROM (SELECT "name", "age" FROM (SELECT "name", "age" FROM "User1") UNION SELECT "name", "age" FROM (SELECT "name", "age" FROM "User2") UNION SELECT "name", "age" FROM (SELECT "name", "age" FROM "User3");) u)");

  EXPECT_EQ(users.size(), 3);
  EXPECT_EQ(users.at(0).name, "Jane");
  EXPECT_EQ(users.at(0).age, 25);
  EXPECT_EQ(users.at(1).name, "Joe");
  EXPECT_EQ(users.at(1).age, 40);
  EXPECT_EQ(users.at(2).name, "John");
  EXPECT_EQ(users.at(2).age, 30);
}

TEST(postgres, test_union_in_join) {
  using namespace sqlgen::literals;

  // Connect to postgres database
  const auto conn = sqlgen::postgres::connect();

  // Create and insert a user
  const auto user1 = User1{.name = "John", .age = 30};
  sqlgen::write(conn, user1);

  const auto user2 = User2{.name = "Jane", .age = 25};
  sqlgen::write(conn, user2);

  const auto user3 = User3{.age = 40, .name = "Joe"};
  sqlgen::write(conn, user3);

  const auto login = Login{.id = 1, .username = "John"};
  sqlgen::write(conn, login);

  const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
  const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);
  const auto s3 = sqlgen::select_from<User3>("name"_c, "age"_c);

  const auto united = sqlgen::unite<std::vector<User1>>(s1, s2, s3);

  const auto sel =
      sqlgen::select_from<Login>(
          "id"_c, "username"_c,
          sqlgen::inner_join(united.as("u"), "username"_c == "u.name"_c))
          .where("id"_c == 1);

  const auto result = sel(conn);

  const auto users = result.value();

  const auto query = sqlgen::postgres::to_sql(sel);

  EXPECT_EQ(
      query,
      R"(SELECT "id", "username" FROM "Login" INNER JOIN (SELECT "name", "age" FROM (SELECT "name", "age" FROM "User1") UNION SELECT "name", "age" FROM (SELECT "name", "age" FROM "User2") UNION SELECT "name", "age" FROM (SELECT "name", "age" FROM "User3");) u ON "username" = u."name" WHERE "id" = 1)");

  EXPECT_EQ(users.size(), 1);
  EXPECT_EQ(users.at(0).id, 1);
  EXPECT_EQ(users.at(0).username, "John");
}

}  // namespace test_unite
