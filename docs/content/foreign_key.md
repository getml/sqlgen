# `sqlgen::ForeignKey`

`sqlgen::ForeignKey` is used to establish referential integrity between tables by creating foreign key relationships. It ensures that values in one table reference valid values in another table's primary key.

## Usage

### Basic Definition

Define a foreign key field in your struct by specifying the type, the referenced table type, and the column name:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    int age;
};

struct Relationship {
    sqlgen::ForeignKey<uint32_t, Person, "id"> parent_id;
    uint32_t child_id;
};
```

This generates the following SQL schema:

```sql
CREATE TABLE IF NOT EXISTS "Person"(
    "id"         INTEGER NOT NULL, 
    "first_name" TEXT NOT NULL, 
    "last_name"  TEXT NOT NULL, 
    "age"        INTEGER NOT NULL,
    PRIMARY KEY("id")
);

CREATE TABLE IF NOT EXISTS "Relationship"(
    "parent_id" INTEGER NOT NULL REFERENCES "Person"("id"),
    "child_id"  INTEGER NOT NULL
);
```

### Template Parameters

The `sqlgen::ForeignKey` template takes three parameters:

1. **T**: The type of the foreign key field (must match the referenced column's type)
2. **ForeignTableType**: The struct type of the table being referenced
3. **col_name**: The name of the column in the referenced table (as a string literal)

```cpp
sqlgen::ForeignKey<Type, ReferencedTable, "column_name"> field_name;
```

**Important**: The referenced column must be a primary key in the foreign table.

### Type Safety and Validation

`sqlgen::ForeignKey` provides compile-time validation to ensure:

1. **Column Existence**: The referenced column must exist in the foreign table
2. **Primary Key Constraint**: The referenced column must be a primary key
3. **Type Compatibility**: The foreign key type must match the referenced column's type

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string name;
};

struct Order {
    sqlgen::PrimaryKey<uint32_t> id;
    // This will compile successfully - "id" exists in Person, is a primary key, and types match
    sqlgen::ForeignKey<uint32_t, Person, "id"> person_id;
        
    // This would cause a compile error - type mismatch
    // sqlgen::ForeignKey<std::string, Person, "id"> person_id;
};
```

### Assignment and Access

Assign values to foreign key fields:

```cpp
const auto relationship = Relationship{
    .parent_id = 1,  // References Person with id = 1
    .child_id = 2
};
```

Access the underlying value using any of these methods:

```cpp
relationship.parent_id();
relationship.parent_id.get();
relationship.parent_id.value();
```

### Working with Related Data

Foreign keys enable you to establish relationships between tables and perform joins:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
};

struct Relationship {
    sqlgen::ForeignKey<uint32_t, Person, "id"> parent_id;
    uint32_t child_id;
};

// Insert parent records
auto people = std::vector<Person>({
    Person{.id = 1, .first_name = "Homer", .last_name = "Simpson"},
    Person{.id = 2, .first_name = "Marge", .last_name = "Simpson"}
});

// Insert relationship records that reference the parents
auto relationships = std::vector<Relationship>({
    Relationship{.parent_id = 1, .child_id = 3},  // Homer -> child 3
    Relationship{.parent_id = 1, .child_id = 4},  // Homer -> child 4
    Relationship{.parent_id = 2, .child_id = 3},  // Marge -> child 3
    Relationship{.parent_id = 2, .child_id = 4}   // Marge -> child 4
});

// Write both tables to the database
conn.and_then(create_table<Person> | if_not_exists)
    .and_then(create_table<Relationship> | if_not_exists)
    .and_then(insert(std::ref(people)))
    .and_then(insert(std::ref(relationships)));
```

### Referential Integrity

Foreign keys enforce referential integrity at the database level:

- **Insert Validation**: You cannot insert a foreign key value that doesn't exist in the referenced table
- **Delete Protection**: You cannot delete a referenced record without handling the foreign key constraints
- **Update Consistency**: Updates to referenced primary keys are handled according to the database's foreign key rules

```cpp
// This would fail if Person with id = 999 doesn't exist
auto invalid_relationship = Relationship{
    .parent_id = 999,  // This Person doesn't exist
    .child_id = 1
};
```

## Notes

- The template parameters are:
  - `T`: The type of the foreign key field
  - `ForeignTableType`: The struct type of the referenced table
  - `col_name`: The name of the referenced column (string literal)
- The class supports:
  - Direct value assignment
  - Multiple access methods for the underlying value
  - Reflection for SQL operations
  - Move and copy semantics
  - Compile-time validation of column existence, primary key constraint, and type compatibility
- Foreign keys can reference any supported SQL data type
- The referenced column must exist in the foreign table, be a primary key, and have a compatible type
- Foreign key relationships are enforced at the database level for data integrity
