# `sqlgen::Unique`

`sqlgen::Unique` is used to enforce uniqueness constraints on table columns. It ensures that no two rows in a table can have the same value for the specified column, providing data integrity at the database level.

## Usage

### Basic Definition

Define a unique field in your struct by wrapping the type with `sqlgen::Unique`:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    sqlgen::Unique<double> age;
};
```

This generates the following SQL schema:

```sql
CREATE TABLE IF NOT EXISTS "Person"(
    "id"         INTEGER NOT NULL, 
    "first_name" TEXT NOT NULL, 
    "last_name"  TEXT NOT NULL, 
    "age"        NUMERIC NOT NULL UNIQUE,
    PRIMARY KEY("id")
);
```

### Template Parameters

The `sqlgen::Unique` template takes one parameter:

1. **T**: The type of the unique field (any supported SQL data type)

```cpp
sqlgen::Unique<Type> field_name;
```

### Type Safety and Validation

`sqlgen::Unique` provides compile-time validation and runtime uniqueness enforcement:

1. **Type Safety**: The wrapped type must be a supported SQL data type
2. **Database-Level Enforcement**: The database enforces uniqueness constraints
3. **Insert Validation**: You cannot insert duplicate values for unique fields

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    sqlgen::Unique<std::string> first_name;  // Each first_name must be unique
    std::string last_name;
    double age;
};
```

### Assignment and Access

Assign values to unique fields:

```cpp
const auto person = Person{
    .id = 1,
    .first_name = "Homer",  // Must be unique across all Person records
    .last_name = "Simpson",
    .age = 45
};
```

Access the underlying value using any of these methods:

```cpp
person.first_name();
person.first_name.get();
person.first_name.value();
```

### Working with Unique Constraints

Unique constraints are useful for ensuring data integrity in various scenarios:

```cpp
struct User {
    sqlgen::PrimaryKey<uint32_t> id;
    sqlgen::Unique<std::string> email;      // Each email must be unique
    sqlgen::Unique<std::string> username;   // Each username must be unique
    std::string full_name;
    std::string password_hash;
};

// Insert user records
auto users = std::vector<User>({
    User{.id = 1, .email = "homer@simpson.com", .username = "homer", .full_name = "Homer Simpson", .password_hash = "hash1"},
    User{.id = 2, .email = "marge@simpson.com", .username = "marge", .full_name = "Marge Simpson", .password_hash = "hash2"}
});

// Write to database - duplicate emails or usernames would cause an error
conn.and_then(create_table<User> | if_not_exists)
    .and_then(insert(std::ref(users)));
```

### Combining with Other Constraints

You can combine `sqlgen::Unique` with other constraints and types:

```cpp
struct Employee {
    sqlgen::PrimaryKey<uint32_t> id;
    sqlgen::Unique<std::string> employee_id;   // Unique employee ID
    sqlgen::Unique<std::string> email;         // Unique email address
    std::string name;
    sqlgen::Unique<double> salary;             // Unique salary (if applicable)
};
```

### Database-Specific Behavior

The generated SQL will be consistent across all supported databases:

```sql
-- PostgreSQL
CREATE TABLE IF NOT EXISTS "Person" ("id" INTEGER NOT NULL, "first_name" TEXT NOT NULL UNIQUE, "last_name" TEXT NOT NULL, "age" NUMERIC NOT NULL, PRIMARY KEY ("id"));

-- MySQL  
CREATE TABLE IF NOT EXISTS `Person` (`id` BIGINT NOT NULL, `first_name` TEXT NOT NULL, `last_name` TEXT NOT NULL, `age` DECIMAL NOT NULL UNIQUE, PRIMARY KEY (`id`));

-- SQLite
CREATE TABLE IF NOT EXISTS "Person" ("id" INTEGER PRIMARY KEY NOT NULL, "first_name" TEXT NOT NULL UNIQUE, "last_name" TEXT NOT NULL, "age" REAL NOT NULL);
```

## Notes

- The template parameter specifies the type of the unique field
- The class supports:
  - Direct value assignment
  - Multiple access methods for the underlying value
  - Reflection for SQL operations
  - Move and copy semantics
  - Compile-time type validation
- Unique constraints can be used with any supported SQL data type
- Database-level enforcement ensures data integrity across concurrent operations
- Unique constraints are useful for business rules like unique emails, usernames, or SKU codes

