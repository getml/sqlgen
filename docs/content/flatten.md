# `sqlgen::Flatten`

The `sqlgen::Flatten` class provides a way to compose database tables by embedding one struct within another, while maintaining a flat database schema. It's particularly useful for creating normalized database structures while keeping a clean C++ interface.

## Usage

### Basic Definition

Define nested structures and use `Flatten` to compose them:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    int age;
};

struct Employee {
    static constexpr const char* tablename = "EMPLOYEES";
    
    sqlgen::Flatten<Person> person;
    float salary;
};
```

This generates a flat SQL schema where the `Person` fields are directly embedded in the `EMPLOYEES` table:

```sql
CREATE TABLE IF NOT EXISTS "EMPLOYEES"(
    "id" INTEGER NOT NULL,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL,
    "salary" REAL NOT NULL,
    PRIMARY KEY("id")
);
```

### Assignment and Access

Assign values to flattened fields:

```cpp
const auto employee = Employee{
    .person = Person{
        .id = 1,
        .first_name = "Homer",
        .last_name = "Simpson",
        .age = 45
    },
    .salary = 60000.0
};
```

Access the nested fields directly:

```cpp
// Access nested fields
employee.person().first_name;  // "Homer"
employee.person().age;         // 45
employee.salary;            // 60000.0
```

### Reading and Writing

Use `Flatten` with `sqlgen::read` and `sqlgen::write` operations:

```cpp
const auto conn = sqlgen::sqlite::connect();

// Write flattened data
sqlgen::write(conn, std::vector<Employee>{employee});

// Read flattened data
const auto employees = sqlgen::read<std::vector<Employee>>(conn).value();
```

## Notes

- `Flatten` is implemented as an alias to `rfl::Flatten<T>`, leveraging the reflection capabilities of reflect-cpp
- The flattened fields are stored directly in the parent table, maintaining a normalized database structure
- When using `Flatten`, the nested struct's fields are treated as if they were direct fields of the parent struct
- Primary keys and other SQL constraints from nested structs are preserved in the generated schema
