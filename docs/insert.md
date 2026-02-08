# `sqlgen::insert`, `sqlgen::insert_or_replace`, `sqlgen::returning`

The `sqlgen::insert` interface provides a type-safe way to insert data from C++ containers or ranges into a SQL database. Unlike `sqlgen::write`, it does not create tables automatically and is designed to be used within transactions. It's particularly useful when you need fine-grained control over table creation and transaction boundaries. 

In particular, `sqlgen::insert` is the recommended way whenever you want to insert data into several tables that depend on each other, meaning that either all inserts should succeed or none of them.

## Usage

### Basic Insert

Insert a container of objects into a database:

```cpp
const auto people = std::vector<Person>({
    Person{.id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10}
});

// Using with a connection reference
const auto conn = sqlgen::sqlite::connect();
sqlgen::insert(conn, people);
```

This generates the following SQL:

```sql
INSERT INTO "Person" ("id", "first_name", "last_name", "age") VALUES (?, ?, ?, ?);
```

Note that `conn` is actually a connection wrapped into an `sqlgen::Result<...>`.
This means you can use monadic error handling and fit this into a single line:

```cpp
// sqlgen::Result<Ref<Connection>>
const auto result = sqlgen::sqlite::connect("database.db").and_then(
                        sqlgen::insert(people));
```

Please refer to the documentation on `sqlgen::Result<...>` for more information on error handling.

### With `Result<Ref<Connection>>`

Handle connection creation and insertion in a single chain:

```cpp
sqlgen::sqlite::connect("database.db")
    .and_then(sqlgen::insert(people))
    .value();
```

### With Iterators

Insert a range of objects using iterators:

```cpp
std::vector<Person> people = /* ... */;
sqlgen::insert(conn, people.begin(), people.end());
```

### With Reference Wrapper

Insert data using a reference wrapper to avoid copying:

```cpp
const auto people = std::vector<Person>(/* ... */);
sqlgen::sqlite::connect("database.db")
    .and_then(sqlgen::insert(std::ref(people)))
    .value();
```

### Conflict Policies (`or_replace`, `or_ignore`)

`insert(...)` supports typed conflict-policy tags:

```cpp
using namespace sqlgen;

insert(people, or_replace);
insert(people, or_ignore);

// Pipeline style is also supported (suggest):
insert(people) | or_replace;
insert(people) | or_ignore;
```

Behavior by backend:

- SQLite: `OR REPLACE`, `OR IGNORE`
- PostgreSQL: `ON CONFLICT (...) DO UPDATE ...`, `ON CONFLICT DO NOTHING`
- DuckDB: `OR REPLACE`, `OR IGNORE`
- MySQL: `ON DUPLICATE KEY UPDATE`, `INSERT IGNORE`

Compile-time rules:

- You can set at most one conflict policy (`or_replace` or `or_ignore`).
- `or_replace` requires at least one primary key or unique constraint.

### Returning Auto-generated IDs (`returning(ids)`)

Use `returning(ids)` to collect auto-generated primary keys during `insert`:

```cpp
struct Person {
  sqlgen::PrimaryKey<uint32_t, sqlgen::auto_incr> id;
  std::string first_name;
  int age;
};

auto ids = std::vector<uint32_t>{};

sqlite::connect()
    .and_then(create_table<Person> | if_not_exists)
    .and_then(insert(people, returning(ids)))
    .value();
```

Compile-time rules:

- The target type must contain an auto-incrementing primary key.
- `returning(ids)` cannot be combined with `or_ignore`.
- The `ids` container must support `clear()` and `push_back(value_type)`.
- On MySQL, `returning(ids)` is supported for single-object inserts only.

Backend behavior:

- SQLite/PostgreSQL/DuckDB: generated SQL uses `RETURNING`.
- MySQL: no `RETURNING` SQL is emitted; IDs are read via the MySQL C API.

### Backward Compatibility (`insert_or_replace`)

`insert_or_replace(...)` is still available and works like before. Internally it is now a thin wrapper over `insert(..., or_replace)`.

## Example: Full Transaction Usage

Here's a complete example showing how to use `insert` within a transaction:

```cpp
using namespace sqlgen;

const auto result = sqlite::connect()
    .and_then(begin_transaction)
    .and_then(create_table<Person> | if_not_exists)
    .and_then(insert(std::ref(people)))
    .and_then(commit)
    .value();
```

This generates the following SQL:

```sql
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "Person" (
    "id" INTEGER PRIMARY KEY,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);
INSERT INTO "Person" ("id", "first_name", "last_name", "age") VALUES (?, ?, ?, ?);
COMMIT;
```

## Example: Multi-Table Insert with Dependencies

Here's an example showing how to insert related data into multiple tables within a single transaction:

```cpp
struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

struct Children {
  uint32_t id_parent;
  sqlgen::PrimaryKey<uint32_t> id_child;
};

// Parent records
const auto people = std::vector<Person>({
    Person{.id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
    Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
    Person{.id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0}
});

// Child relationships
const auto children = std::vector<Children>({
    Children{.id_parent = 0, .id_child = 1},  // Homer -> Bart
    Children{.id_parent = 0, .id_child = 2},  // Homer -> Lisa
    Children{.id_parent = 0, .id_child = 3}   // Homer -> Maggie
});

using namespace sqlgen;

// All inserts are wrapped in a single transaction
const auto result = postgres::connect(credentials)
    .and_then(begin_transaction)
    .and_then(create_table<Person> | if_not_exists)
    .and_then(create_table<Children> | if_not_exists)
    .and_then(insert(std::ref(people)))    // Insert people first
    .and_then(insert(std::ref(children)))  // Then insert relationships
    .and_then(commit)
    .value();
```

This generates the following SQL:

```sql
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "Person" (
    "id" INTEGER PRIMARY KEY,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS "Children" (
    "id_parent" INTEGER NOT NULL,
    "id_child" INTEGER PRIMARY KEY
);
INSERT INTO "Person" ("id", "first_name", "last_name", "age") VALUES (?, ?, ?, ?);
INSERT INTO "Children" ("id_parent", "id_child") VALUES (?, ?);
COMMIT;
```

In this example, we insert both parent records and their relationships to children. If any insert fails, the entire transaction is rolled back, ensuring data consistency. This is crucial when dealing with related data across multiple tables.

## Comparison with `sqlgen::write`

While both `insert` and `write` can be used to add data to a database, they serve different purposes:

### `sqlgen::write`
- Automatically creates tables if they don't exist
- Uses optimized bulk insert methods (e.g., PostgreSQL's COPY command)
- More convenient for simple use cases
- Faster for bulk inserts
- Cannot be used within transactions
- Best for single-table operations with no dependencies

### `sqlgen::insert`
- Does not create tables automatically
- Uses standard INSERT statements
- Designed to be used within transactions
- More control over transaction boundaries
- Better for complex operations requiring transaction support
- Essential for multi-table operations with dependencies
- Guarantees atomicity across multiple tables
- Recommended when data consistency across tables is critical

## Notes

- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed
- `insert(...)` accepts optional modifiers: `or_replace`, `or_ignore`, `returning(ids)`
- Modifiers can be passed directly (`insert(data, or_replace)`) or in pipeline style (`insert(data) | or_replace`)
- Unlike `write`, `insert` does not create tables automatically - you must create tables separately using `create_table`
- The insert operation is atomic within a transaction
- When using reference wrappers (`std::ref`), the data is not copied, which can be more efficient for large datasets
- On MySQL, `returning(ids)` is limited to single-object inserts
