# `sqlgen::write`

The `sqlgen::write` interface provides a type-safe way to write data from C++ containers or ranges to a SQL database. It handles table creation, batch processing, and error handling automatically.

## Usage

### Basic Write

Write a container of objects to a database:

```cpp
const auto people = std::vector<Person>({
    Person{.id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10}
});

// Using with a connection reference
const auto conn = sqlgen::sqlite::connect();
sqlgen::write(conn, people);
```

This generates the following SQL for PostgreSQL:

```sql
CREATE TABLE IF NOT EXISTS "Person" (
    "id" INTEGER,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);

COPY "Person"("id", "first_name", "last_name", "age") FROM STDIN WITH DELIMITER '\t' NULL '\e' QUOTE '\a';
```

For other dialects like SQLite, it uses prepared INSERT statements instead:

```sql
CREATE TABLE IF NOT EXISTS "Person" (
    "id" INTEGER,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);

INSERT INTO "Person" ("id", "first_name", "last_name", "age") VALUES (?, ?, ?, ?);
```

### With Result<Ref<Connection>>

Handle connection creation and writing in a single chain:

```cpp
sqlgen::sqlite::connect("database.db")
    .and_then([&](auto&& conn) { return sqlgen::write(conn, people); })
    .value();
```

This generates the same SQL as above, adapted to the specific database dialect being used.

### With Iterators

Write a range of objects using iterators:

```cpp
std::vector<Person> people = /* ... */;
sqlgen::write(conn, people.begin(), people.end());
```

This also generates the same SQL, adapted to the specific database dialect.

### Curried Write

You can also use a curried version of `write` that takes the data first and returns a function that accepts the connection. This is useful for chaining operations:

```cpp
const auto people = std::vector<Person>({
    Person{.id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10}
});

// Using with a connection reference
const auto conn = sqlgen::sqlite::connect();
sqlgen::write(people)(conn);  // Creates a deep copy of people

// To avoid deep copy, use std::ref
sqlgen::write(std::ref(people))(conn);  // Passes people by reference

// Or in a chain with other operations
sqlgen::sqlite::connect()
    .and_then(sqlgen::write(std::ref(people)))  // Pass data by reference
    .and_then(sqlgen::read<std::vector<Person>>)
    .value();
```

Note that by default, the curried `write` will create a deep copy of your data. If you want to avoid this overhead, wrap your data in `std::ref` when passing it to `write`. This is especially important for large datasets.

The curried version is particularly useful when you want to:
- Chain multiple database operations together
- Pass the write operation as a function to other operations

## How It Works

The `write` function performs the following operations in sequence:

1. Creates a table if it doesn't exist (using the object's structure)
2. Prepares an insert statement
3. Writes the data in batches of `SQLGEN_BATCH_SIZE`, which you can set at compile time
4. Handles any errors that occur during the process

## Notes

- The function automatically creates the table, if it doesn't exist
- Data is written in batches for better performance
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed
- The function has three overloads:
  1. Takes a connection reference and iterators
  2. Takes a `Result<Ref<Connection>>` and iterators
  3. Takes a connection and a container directly
- The SQL generation adapts to the database dialect:
  - PostgreSQL uses the efficient COPY command for bulk inserts
  - Other dialects use prepared INSERT statements

