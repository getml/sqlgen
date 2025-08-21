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

