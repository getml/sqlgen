# Defining tables

`sqlgen` provides a type-safe way to define database tables using C++ structs. It automatically handles table creation, schema management, and nullability through C++ types.

## Usage

### Basic Table Definition

Define a table using a simple C++ struct:

```cpp
struct People {
    std::string first_name;
    std::string last_name;
    uint age;
};
```

When you write data to this table, it will be automatically created if it doesn't exist:

```cpp
const auto people = std::vector<People>(...);
const auto result = sqlgen::write(conn, people);
```

This generates SQL similar to:

```sql
CREATE TABLE IF NOT EXISTS "People"(
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);
```

### Custom Table Names

Override the default table name using a static member:

```cpp
struct People {
    constexpr static const char* tablename = "PEOPLE";
    
    std::string first_name;
    std::string last_name;
    uint age;
};
```

This generates SQL with the custom table name:

```sql
CREATE TABLE IF NOT EXISTS "PEOPLE"(
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);
```

### Custom Schemas

Define a table within a specific schema:

```cpp
struct People {
    constexpr static const char* schema = "my_schema";
    
    std::string first_name;
    std::string last_name;
    uint age;
};
```

This generates SQL with the schema specification:

```sql
CREATE TABLE IF NOT EXISTS "my_schema"."People"(
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL
);
```

### Nullable Fields

As we have seen, all columns are non-nullable by default. But what if we do want
nullability?

As with reflect-cpp, `std::optional`, `std::shared_ptr` and `std::unique_ptr` are interpreted as nullable types,
but it is strongly recommended that you use `std::optional`.

So you can create nullable fields as follows:

```cpp
struct People {
    rfl::PrimaryKey<std::string> first_name;
    std::string last_name;
    std::optional<uint> age;
};
```

## Notes

- All fields are non-nullable by default
- Nullable fields can be defined using:
  - `std::optional<T>` (strongly recommended)
  - `std::shared_ptr<T>`
  - `std::unique_ptr<T>`
- Use `std::nullopt` or `nullptr` to represent NULL values
- The exact SQL generated may vary between database dialects
- Table names are inferred from struct names by default
- Custom table names and schemas can be specified using static members
