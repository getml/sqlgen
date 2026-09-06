# `sqlgen::drop` 

The `sqlgen::drop` interface provides a type-safe way to drop tables from a SQL database. It supports the `if_exists` clause to handle cases where the table may not exist.

## Usage

### Basic Drop

Drop a table:

```cpp
const auto conn = sqlgen::sqlite::connect("database.db");

sqlgen::drop<Person>(conn).value();
```

This generates the following SQL:

```sql
DROP TABLE "Person";
```

Note that `conn` is actually a connection wrapped into an `sqlgen::Result<...>`.
This means you can use monadic error handling and fit this into a single line:

```cpp
// sqlgen::Result<Ref<Connection>>
const auto result = sqlgen::sqlite::connect("database.db").and_then(
                        sqlgen::drop<Person>);
```

Please refer to the documentation on `sqlgen::Result<...>` for more information on error handling.

### With `if_exists` clause

Drop a table only if it exists:

```cpp
using namespace sqlgen;

const auto query = drop<Person> | if_exists;

query(conn).value();
```

This generates the following SQL:

```sql
DROP TABLE IF EXISTS "Person";
```

You can also use monadic error handling here:

```cpp
using namespace sqlgen;

const auto query = drop<Person> | if_exists;

// sqlgen::Result<Ref<Connection>>
const auto result = sqlite::connect("database.db").and_then(query);
```

## Example: Full Query Composition

```cpp
using namespace sqlgen;

const auto query = drop<Person> | if_exists;

const auto result = query(conn);

if (!result) {
    // Error handling
}
```

This generates the following SQL:

```sql
DROP TABLE IF EXISTS "Person";
```

It is strongly recommended that you use `using namespace sqlgen`. However,
if you do not want to do that, you can rewrite the example above as follows:

```cpp
const auto query = sqlgen::drop<Person> | sqlgen::if_exists;

const auto result = query(conn);
```

## Notes

- The `if_exists` clause is optional - if omitted, the query will fail if the table doesn't exist
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed or refer to the documentation on `sqlgen::Result<...>` for other forms of error handling.
- The table name is derived from the struct name (e.g., `Person` becomes `"Person"` in SQL)

