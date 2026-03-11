#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

namespace test_insert_returning_ids {

struct Person {
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
  std::string first_name;
  int age;
};

TEST(postgres, test_insert_returning_ids) {
  const auto people =
      std::vector<Person>({Person{.first_name = "Homer", .age = 45},
                           Person{.first_name = "Bart", .age = 10},
                           Person{.first_name = "Lisa", .age = 8}});

  auto ids = std::vector<uint32_t>{};

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto people_from_db =
      postgres::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(create_table<Person> | if_not_exists)
          .and_then(insert(std::ref(people), returning(ids)))
          .and_then(sqlgen::read<std::vector<Person>> | order_by("id"_c))
          .value();

  ASSERT_EQ(ids.size(), people.size());
  EXPECT_EQ(ids, (std::vector<uint32_t>{1, 2, 3}));

  auto ids_from_db = std::vector<uint32_t>{};

  for (const auto& person : people_from_db) {
    ids_from_db.push_back(person.id());
  }

  EXPECT_EQ(ids, ids_from_db);
}

}  // namespace test_insert_returning_ids

#endif
