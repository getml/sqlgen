# ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) sqlgen 

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/getml/reflect-cpp/graphs/commit-activity)
[![Generic badge](https://img.shields.io/badge/C++-20-blue.svg)](https://shields.io/)
[![Generic badge](https://img.shields.io/badge/gcc-11+-blue.svg)](https://shields.io/)
[![Generic badge](https://img.shields.io/badge/clang-14+-blue.svg)](https://shields.io/)
[![Generic badge](https://img.shields.io/badge/MSVC-17+-blue.svg)](https://shields.io/)

sqlgen is a modern, type-safe ORM and SQL query generator for C++20, inspired by Python's [SQLAlchemy](https://github.com/sqlalchemy/sqlalchemy)/[SQLModel](https://github.com/fastapi/sqlmodel) and Rust's [Diesel](https://github.com/diesel-rs/diesel). It provides a fluent, composable interface for database operations with compile-time type checking and SQL injection protection.

sqlgen is based on and tightly integrated with [reflect-cpp](https://github.com/getml/reflect-cpp), a C++-20 library for fast serialization, deserialization and validation using reflection, similar to pydantic in Python, serde in Rust, encoding in Go or aeson in Haskell.

Together, reflect-cpp and sqlgen enable reliable and efficient ETL pipelines.

## Features

- 🔒 **Type Safety**: Compile-time validation of table schemas and queries
- 🛡️ **SQL Injection Protection**: Built-in input validation and parameterized queries
- 🔄 **Composable Queries**: Fluent interface for building complex queries
- 🚀 **High Performance**: Efficient batch operations and prepared statements
- 📦 **Modern C++**: Leverages C++20 features for a clean, expressive API
- 🔌 **Multiple Backends**: Support for PostgreSQL and SQLite
- 🔍 **Reflection Integration**: Seamless integration with [reflect-cpp](https://github.com/getml/reflect-cpp) 

## Quick Start

### Installation

1. Make sure you have the required dependencies installed (skip this step on Windows):
```bash
sudo apt-get install autoconf bison flex # Linux (Ubuntu, Debian, ...)
brew install autoconf bison flex # macOS
```

2. Set up vcpkg:
```bash
git submodule update --init
./vcpkg/bootstrap-vcpkg.sh  # Linux, macOS
./vcpkg/bootstrap-vcpkg.bat # Windows
```

3. Build the library:
```bash
cmake -S . -B build -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4  # gcc, clang
cmake --build build --config Release -j 4  # MSVC
```

4. Include in your CMake project:
```cmake
find_package(sqlgen REQUIRED)
target_link_libraries(your_target PRIVATE sqlgen::sqlgen)
```

## Usage Examples

### Hello World

```cpp
#include <sqlgen/sqlite.hpp>
#include <iostream>

struct User {
    std::string name;
    int age;
};

int main() {
    // Connect to SQLite database
    const auto conn = sqlgen::sqlite::connect("test.db");
    
    // Create and insert a user
    const auto user = User{.name = "John", .age = 30};
    sqlgen::write(conn, user);
    
    // Read all users
    const auto users = sqlgen::read<std::vector<User>>(conn).value();
    
    for (const auto& u : users) {
        std::cout << u.name << " is " << u.age << " years old\n";
    }
}
```

### Connecting to a Database

```cpp
#include <sqlgen/postgres.hpp>

// PostgreSQL connection
const auto credentials = sqlgen::postgres::Credentials{
    .user = "username",
    .password = "password",
    .host = "localhost",
    .dbname = "mydb",
    .port = 5432
};

const auto conn = sqlgen::postgres::connect(credentials);

// SQLite connection
const auto sqlite_conn = sqlgen::sqlite::connect("database.db");
```

### Defining Models

```cpp
struct Person {
    std::string first_name;
    std::string last_name;
    uint32_t age;
    std::optional<std::string> email;  // Nullable field
};
```

### Inserting Data

```cpp
const auto people = std::vector<Person>({
    Person{.first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.first_name = "Marge", .last_name = "Simpson", .age = 42}
});

// Automatically creates table if it doesn't exist
const auto result = sqlgen::write(conn, people);

if (!result) {
    std::cerr << "Error: " << result.error().what() << std::endl;
}
```

Generated SQL:
```sql
CREATE TABLE IF NOT EXISTS "Person" (
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL,
    "email" TEXT
);

INSERT INTO "Person" ("first_name", "last_name", "age", "email") 
VALUES (?, ?, ?, ?);
```

### Querying Data

```cpp
#include <rfl/json.hpp>
#include <sqlgen/postgres.hpp>

using namespace sqlgen;

// Build a query for adults, ordered by age
const auto query = read<std::vector<Person>> |
                   where("age"_c >= 18) |
                   order_by("age"_c.desc(), "last_name"_c) |
                   limit(10);

// Execute the query
const auto result = query(conn);

if (result) {
    // Print results as JSON
    std::cout << rfl::json::write(*result) << std::endl;
} else {
    std::cerr << "Error: " << result.error().what() << std::endl;
}
```

Generated SQL:
```sql
SELECT "first_name", "last_name", "age", "email"
FROM "Person"
WHERE "age" >= 18
ORDER BY "age" DESC, "last_name"
LIMIT 10;
```

### Grouping and Aggregating Data

```cpp
using namespace sqlgen;

struct Children {
    std::string last_name;
    int num_children;
    int max_age;
    int min_age;
    int sum_age;
};

const auto get_children = select_from<Person>(
    "last_name"_c,
    count().as<"num_children">(),
    max("age"_c).as<"max_age">(),
    min("age"_c).as<"min_age">(),
    sum("age"_c).as<"sum_age">(),
) | where("age"_c < 18) | group_by("last_name"_c) | to<std::vector<Children>>;

const std::vector<Children> children = get_children(conn).value();
```

Generated SQL:
```sql
SELECT 
    "last_name",
    COUNT(*) as "num_children",
    MAX("age") as "max_age",
    MIN("age") as "min_age",
    SUM("age") as "sum_age"
FROM "Person"
WHERE "age" < 18
GROUP BY "last_name";
```

### Type Safety and SQL Injection Protection

sqlgen provides comprehensive compile-time checks and runtime protection:

```cpp
// Compile-time error: No such column "color"
const auto query = read<std::vector<Person>> |
                   where("color"_c == "blue");

// Compile-time error: Cannot compare column "age" to a string 
const auto query = read<std::vector<Person>> |
                   where("age"_c == "Homer");

// Runtime protection against SQL injection
std::vector<Person> get_people(const auto& conn, 
                              const sqlgen::AlphaNumeric& first_name) {
    using namespace sqlgen;
    return (read<std::vector<Person>> | 
            where("first_name"_c == first_name))(conn).value();
}

// This will be rejected
get_people(conn, "Homer' OR '1'='1");  // SQL injection attempt
```

## Documentation

For detailed documentation, visit our [documentation page](docs/README.md).

## Contributing

We welcome constructive criticism, feature requests and contributions! Please open an issue or a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
