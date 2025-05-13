# `sqlgen::update` 

The `sqlgen::update` interface provides a type-safe way to update records in a SQL database. It supports composable query building with `where` clauses to specify which records should be updated, and allows setting multiple columns to either literal values or other column values.

## Usage

### Basic Update

Update specific columns in a table:

```cpp
const auto conn = sqlgen::sqlite::connect("database.db");

using namespace sqlgen;

const auto query = update<Person>("age"_c.set(100), "first_name"_c.set("New Name"));

query(conn).value();
```

This generates the following SQL:

```sql
UPDATE "Person" 
SET "age" = 100, "first_name" = 'New Name';
```

Note that `conn` is actually a connection wrapped into an `sqlgen::Result<...>`.
This means you can use monadic error handling and fit this into a single line:

```cpp
// sqlgen::Result<Ref<Connection>>
const auto result = sqlgen::sqlite::connect("database.db").and_then(
                        update<Person>("age"_c.set(100), "first_name"_c.set("New Name")));
```

Please refer to the documentation on `sqlgen::Result<...>` for more information on error handling.

### With `where` clause

Update specific records using a `where` clause:

```cpp
using namespace sqlgen;

const auto query = update<Person>("age"_c.set(100)) |
                   where("first_name"_c == "Hugo");

query(conn).value();
```

This generates the following SQL:

```sql
UPDATE "Person"
SET "age" = 100
WHERE "first_name" = 'Hugo';
```

Note that `"..."_c` refers to the name of the column. If such a field does not
exist on the struct `Person`, the code will fail to compile.

You can also use monadic error handling here:

```cpp
using namespace sqlgen;

const auto query = update<Person>("age"_c.set(100)) |
                   where("first_name"_c == "Hugo");

// sqlgen::Result<Ref<Connection>>
const auto result = sqlite::connect("database.db").and_then(query);
```

### Setting Columns to Other Column Values

You can set a column's value to another column's value:

```cpp
using namespace sqlgen;

const auto query = update<Person>("first_name"_c.set("last_name"_c)) |
                   where("age"_c > 18);

query(conn).value();
```

This generates the following SQL:

```sql
UPDATE "Person"
SET "first_name" = "last_name"
WHERE "age" > 18;
```

## Example: Full Query Composition

```cpp
using namespace sqlgen;

const auto query = update<Person>(
                    "first_name"_c.set("last_name"_c),
                    "age"_c.set(100)) |
                   where("age"_c > 0);

const auto result = query(conn);

if (!result) {
    // Error handling
}
```

This generates the following SQL:

```sql
UPDATE "Person"
SET 
    "first_name" = "last_name",
    "age" = 100
WHERE "age" > 0;
```

It is strongly recommended that you use `using namespace sqlgen`. However,
if you do not want to do that, you can rewrite the example above as follows:

```cpp
const auto query = sqlgen::update<Person>(
                    sqlgen::col<"first_name">.set(sqlgen::col<"last_name">),
                    sqlgen::col<"age">.set(100)) |
                   sqlgen::where(sqlgen::col<"age"> > 0);

const auto result = query(conn);
```

## Notes

- You must specify at least one column to update
- The `where` clause is optional - if omitted, all records will be updated
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed or refer to the documentation on `sqlgen::Result<...>` for other forms of error handling
- `"..."_c` refers to the name of the column
- You can set columns to either literal values or other column values
- The update operation is atomic - either all specified columns are updated or none are

