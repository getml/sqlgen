# `sqlgen::create_index` 

The `sqlgen::create_index` interface provides a type-safe way to create indexes on SQL database tables. It supports creating both regular and unique indexes, with optional `if_not_exists` clause to handle cases where the index may already exist.

## Usage

### Basic Index Creation

Create an index on one or more columns:

```cpp
const auto conn = sqlgen::sqlite::connect("database.db");

sqlgen::create_index<"person_ix", Person>("first_name"_c, "last_name"_c)(conn).value();
```

This generates the following SQL:

```sql
CREATE INDEX "person_ix" ON "Person" ("first_name", "last_name");
```

Note that `conn` is actually a connection wrapped into an `sqlgen::Result<...>`.
This means you can use monadic error handling and fit this into a single line:

```cpp
// sqlgen::Result<Ref<Connection>>
const auto result = sqlgen::sqlite::connect("database.db").and_then(
                        sqlgen::create_index<"person_ix", Person>("first_name"_c, "last_name"_c));
```

Please refer to the documentation on `sqlgen::Result<...>` for more information on error handling.

### With `if_not_exists` clause

Create an index only if it doesn't already exist:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = create_index<"person_ix", Person>("first_name"_c, "last_name"_c) | if_not_exists;

query(conn).value();
```

This generates the following SQL:

```sql
CREATE INDEX IF NOT EXISTS "person_ix" ON "Person" ("first_name", "last_name");
```

You can also use monadic error handling here:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = create_index<"person_ix", Person>("first_name"_c, "last_name"_c) | if_not_exists;

// sqlgen::Result<Ref<Connection>>
const auto result = sqlite::connect("database.db").and_then(query);
```

### Creating Unique Indexes

Create a unique index to enforce uniqueness constraints:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = create_unique_index<"person_ix", Person>("first_name"_c, "last_name"_c);

query(conn).value();
```

This generates the following SQL:

```sql
CREATE UNIQUE INDEX "person_ix" ON "Person" ("first_name", "last_name");
```

### Creating Indexes with WHERE Clause

Create a partial index by adding a WHERE clause to filter which rows are included in the index:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = create_index<"test_table_ix", TestTable>("field1"_c, "field2"_c) | 
                  if_not_exists | 
                  where("field2"_c > 0);

query(conn).value();
```

This generates the following SQL:

```sql
CREATE INDEX IF NOT EXISTS "test_table_ix" ON "TestTable"("field1", "field2") WHERE "field2" > 0;
```

The WHERE clause can be combined with other modifiers like `if_not_exists` and works with both regular and unique indexes. This is particularly useful for creating partial indexes that only include rows matching certain conditions, which can improve index performance and reduce index size.

## Example: Full Query Composition

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = create_index<"person_ix", Person>("first_name"_c, "last_name"_c) | if_not_exists;

const auto result = query(conn);

if (!result) {
    // Error handling
}
```

This generates the following SQL:

```sql
CREATE INDEX IF NOT EXISTS "person_ix" ON "Person" ("first_name", "last_name");
```

It is strongly recommended that you use `using namespace sqlgen` and  `using namespace sqlgen::literals`. However,
if you do not want to do that, you can rewrite the example above as follows:

```cpp
const auto query = sqlgen::create_index<"person_ix", Person>("first_name"_c, "last_name"_c) | 
                   sqlgen::if_not_exists;

const auto result = query(conn);
```

## Notes

- The `if_not_exists` clause is optional - if omitted, the query will fail if the index already exists
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed or refer to the documentation on `sqlgen::Result<...>` for other forms of error handling.
- `"..."_c` refers to the name of the column. If such a field does not exist on the struct (e.g., `Person`), the code will fail to compile. It is defined in the namespace `sqlgen::literals`.
- Index names must be unique within a database
- You can create indexes on multiple columns by providing multiple column names
- Use `create_unique_index` when you need to enforce uniqueness constraints on the indexed columns

