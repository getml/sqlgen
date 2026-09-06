# `sqlgen::create_table` 

The `sqlgen::create_table` interface provides a type-safe way to create tables in a SQL database. It supports the `if_not_exists` clause to handle cases where the table may already exist.

## Usage

### Basic Create Table

Create a table:

```cpp
const auto conn = sqlgen::sqlite::connect("database.db");

sqlgen::create_table<Person>(conn).value();
```

This generates the following SQL:

```sql
CREATE TABLE "Person" ("id" INTEGER NOT NULL, "first_name" TEXT NOT NULL, "last_name" TEXT NOT NULL, "age" INTEGER NOT NULL);
```

Note that `conn` is actually a connection wrapped into an `sqlgen::Result<...>`.
This means you can use monadic error handling and fit this into a single line:

```cpp
// sqlgen::Result<Ref<Connection>>
const auto result = sqlgen::sqlite::connect("database.db").and_then(
                        sqlgen::create_table<Person>);
```

Please refer to the documentation on `sqlgen::Result<...>` for more information on error handling.

### With `if_not_exists` clause

Create a table only if it doesn't exist:

```cpp
using namespace sqlgen;

const auto query = create_table<Person> | if_not_exists;

query(conn).value();
```

This generates the following SQL:

```sql
CREATE TABLE IF NOT EXISTS "Person" ("id" INTEGER NOT NULL, "first_name" TEXT NOT NULL, "last_name" TEXT NOT NULL, "age" INTEGER NOT NULL);
```

You can also use monadic error handling here:

```cpp
using namespace sqlgen;

const auto query = create_table<Person> | if_not_exists;

// sqlgen::Result<Ref<Connection>>
const auto result = sqlite::connect("database.db").and_then(query);
```

## Example: Full Query Composition

```cpp
using namespace sqlgen;

const auto query = create_table<Person> | if_not_exists;

const auto result = query(conn);

if (!result) {
    // Error handling
}
```

This generates the following SQL:

```sql
CREATE TABLE IF NOT EXISTS "Person" ("id" INTEGER NOT NULL, "first_name" TEXT NOT NULL, "last_name" TEXT NOT NULL, "age" INTEGER NOT NULL);
```

It is strongly recommended that you use `using namespace sqlgen`. However,
if you do not want to do that, you can rewrite the example above as follows:

```cpp
const auto query = sqlgen::create_table<Person> | sqlgen::if_not_exists;

const auto result = query(conn);
```

## Notes

- The `if_not_exists` clause is optional - if omitted, the query will fail if the table already exists
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed or refer to the documentation on `sqlgen::Result<...>` for other forms of error handling.
- The table name is derived from the struct name (e.g., `Person` becomes `"Person"` in SQL)
- Column types are automatically mapped from C++ types:
  - `std::string` → `TEXT`
  - `int`/`int32_t` → `INTEGER`
  - `uint32_t` → `INTEGER`
  - `std::optional<T>` → nullable column of type T
  - `sqlgen::PrimaryKey<T>` → column of type T with PRIMARY KEY constraint
- All columns are NOT NULL by default unless they are `std::optional`

