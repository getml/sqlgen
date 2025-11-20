# `sqlgen::Dynamic`

`sqlgen::Dynamic` lets you define custom SQL types that aren't natively supported by sqlgen. It works by returning a `sqlgen::dynamic::types::Dynamic` with a database type name string that the transpiler passes directly to the target database.

## Usage

### Parser specialization for boost::uuids::uuid

In this example, we demonstrate how you can use boost::uuids::uuid to automatically generate primary keys. This is not officially supported by the sqlgen library,
but it is very easy to build something like this using `sqlgen::Dynamic`.

The first step is to specialize the `sqlgen::parsing::Parser` for `boost::uuids::uuid` and implement `read`, `write`, and `to_type`:

```cpp
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sqlgen/dynamic/types.hpp>
#include <sqlgen/parsing/Parser.hpp>

namespace sqlgen::parsing {

template <>
struct Parser<boost::uuids::uuid> {
  using Type = boost::uuids::uuid;

  static Result<boost::uuids::uuid> read(
      const std::optional<std::string>& _str) {
    if (!_str) {
      return error("boost::uuids::uuid cannot be NULL.");
    }
    return boost::lexical_cast<boost::uuids::uuid>(*_str);
  }

  static std::optional<std::string> write(
      const boost::uuids::uuid& _u) {
    return boost::uuids::to_string(_u);
  }

  static dynamic::Type to_type() noexcept {
    return sqlgen::dynamic::types::Dynamic{"UUID"};
  }
};

}  // namespace sqlgen::parsing
```

### DuckDB parser specialization

**Important:** If you're using DuckDB, you must also implement a separate parser specialization in the `sqlgen::duckdb::parsing` namespace. This is required for performance reasons, as DuckDB uses its own native types and appender interface.

The DuckDB parser has a different interface than the generic parser:

```cpp
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sqlgen/duckdb.hpp>
#include <exception>

namespace sqlgen::duckdb::parsing {

template <>
struct Parser<boost::uuids::uuid> {
  using ResultingType = duckdb_string_t;

  static Result<boost::uuids::uuid> read(const ResultingType* _r) noexcept {
    return Parser<std::string>::read(_r).and_then(
        [&](const std::string& _str) -> Result<boost::uuids::uuid> {
          try {
            return boost::lexical_cast<boost::uuids::uuid>(_str);
          } catch (const std::exception& e) {
            return error(e.what());
          }
        });
  }

  static Result<Nothing> write(const boost::uuids::uuid& _u,
                               duckdb_appender _appender) noexcept {
    return Parser<std::string>::write(boost::uuids::to_string(_u), _appender);
  }
};

}  // namespace sqlgen::duckdb::parsing
```

Key differences from the generic parser:
- `read` takes `const ResultingType*` (where `ResultingType = duckdb_string_t`) instead of `const std::optional<std::string>&`
- `write` takes a `duckdb_appender` parameter and returns `Result<Nothing>` instead of `std::optional<std::string>`
- No `to_type()` method is required (the generic parser's `to_type()` is used for schema generation)

The second step is to specialize `sqlgen::transpilation::ToValue` for `boost::uuids::uuid` and implement `operator()`:

```cpp
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sqlgen/dynamic/types.hpp>
#include <sqlgen/transpilation/to_value.hpp>

namespace sqlgen::transpilation {

template <>
struct ToValue<boost::uuids::uuid> {
  dynamic::Value operator()(const boost::uuids::uuid& _u) const {
    return dynamic::Value{dynamic::String{.val = boost::uuids::to_string(_u)}};
  }
};

}  // namespace sqlgen::transpilation
```

This second step is necessary to ensure you can use your type in `where(...)` statements
and other conditions.

The return type must always be `sqlgen::dynamic::Value`. `dynamic::Value` can contain
`dynamic::String` (as in this example), `dynamic::Float` or `dynamic::Integer`.

### Using `boost::uuids::uuid` in structs

You can then automatically generate random UUIDs:

```cpp
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

struct Person {
  sqlgen::PrimaryKey<boost::uuids::uuid> id =
      boost::uuids::uuid(boost::uuids::random_generator()());
  std::string first_name;
  std::string last_name;
  int age;
};
```

This generates SQL schema with the custom UUID type name:

```sql
CREATE TABLE IF NOT EXISTS "Person"(
    "id"         UUID NOT NULL,
    "first_name" TEXT NOT NULL,
    "last_name"  TEXT NOT NULL,
    "age"        INTEGER NOT NULL,
    PRIMARY KEY("id")
);
```

### Working with UUIDs

Note that you do not have to assign the UUIDs - this is done automatically:

```cpp
// Create and insert records
std::vector<Person> people = {
  Person{.first_name = "Homer",  .last_name = "Simpson", .age = 45},
  Person{.first_name = "Marge",  .last_name = "Simpson", .age = 42},
  Person{.first_name = "Bart",   .last_name = "Simpson", .age = 10},
  Person{.first_name = "Lisa",   .last_name = "Simpson", .age = 8},
  Person{.first_name = "Maggie", .last_name = "Simpson", .age = 0}
};

// Create table and write
auto res = conn.and_then(sqlgen::create_table<Person> | sqlgen::if_not_exists)
               .and_then(sqlgen::insert(std::ref(people)));

// Read back ordered by age
using namespace sqlgen::literals;
const auto people2 = res.and_then(sqlgen::read<std::vector<Person>> |
                                  sqlgen::order_by("age"_c.desc()))
                        .value();

// Filtering by UUID
const auto target = boost::lexical_cast<boost::uuids::uuid>(
    "550e8400-e29b-41d4-a716-446655440000");
const auto query = sqlgen::read<std::vector<Person>> |
                   sqlgen::where("id"_c == target);
```

## Per-database type name for UUID

You may want to map `boost::uuids::uuid` to different database type names per dialect. Implement `to_type()` accordingly. The tests demonstrate these mappings:

- SQLite:
```cpp
static dynamic::Type to_type() noexcept {
  return sqlgen::dynamic::types::Dynamic{"TEXT"};
}
```

- PostgreSQL:
```cpp
static dynamic::Type to_type() noexcept {
  return sqlgen::dynamic::types::Dynamic{"UUID"};
}
```

- MySQL:
```cpp
static dynamic::Type to_type() noexcept {
  return sqlgen::dynamic::types::Dynamic{"VARCHAR(36)"};
}
```

- DuckDB:
```cpp
static dynamic::Type to_type() noexcept {
  return sqlgen::dynamic::types::Dynamic{"TEXT"};
}
```

Note: For DuckDB, you must also implement the `sqlgen::duckdb::parsing::Parser` specialization as shown in the DuckDB parser specialization section above.

## Parser specialization requirements

Specializing `sqlgen::parsing::Parser<T>` requires three methods. These guidelines help ensure correctness and portability:

1) read
```cpp
static Result<T> read(const std::optional<std::string>& dbValue);
```
- Responsibility: Convert from DB string (or null) to `T`.
- Null handling: If your field cannot be null, return `error("... cannot be NULL.")` when `dbValue` is `std::nullopt`.
- Validation: Parse and validate strictly. Return a descriptive error for malformed input.
- Normalization: If your string form can vary (case, hyphens), normalize consistently so `write(read(x))` is stable.

2) write
```cpp
static std::optional<std::string> write(const T& value);
```
- Responsibility: Convert `T` to the string the DB expects.
- Nulls: Return `std::nullopt` only if you intend to write SQL NULL.
- Round-trip: Aim for `read(write(v)) == v` (modulo normalization).

3) to_type
```cpp
static dynamic::Type to_type() noexcept;
```
- Responsibility: Provide the DB type name via `sqlgen::dynamic::types::Dynamic{"TYPE_NAME"}`.
- Dialect mapping: Choose a valid name for your target DB (e.g., `UUID` on PostgreSQL, `VARCHAR(36)` on MySQL, `TEXT` on SQLite for UUIDs).
- Column properties: Constraints like primary key, unique, and nullability are typically controlled by field wrappers (`sqlgen::PrimaryKey`, `sqlgen::Unique`, `std::optional<T>`). If you are building fully dynamic schemas, you may also set properties on `Dynamic`.

Additional best practices:
- Error messages: Keep them clear and specific to aid debugging.
- Performance: Prefer lightweight conversions in `read`/`write`; avoid expensive allocations inside hot loops.
- Testing: Add round-trip tests (insert/read/compare) like the provided UUID tests across dialects.
- Consistency: Ensure the chosen DB type name matches any indexes, constraints, and length limits you rely on.

## Notes

- `sqlgen::dynamic::types::Dynamic` has:
  - `type_name`: SQL type name string
  - `properties`: column properties (nullable, unique, primary, auto_incr, foreign_key_reference)
- Works with all operations: `create_table`, `insert`, `select`, `update`, `delete`
- The type name is passed directly to the database; ensure it is valid for the target dialect
- Keep specializations in the `sqlgen::parsing` namespace
- **DuckDB users:** You must implement both `sqlgen::parsing::Parser` and `sqlgen::duckdb::parsing::Parser` specializations for your custom type

