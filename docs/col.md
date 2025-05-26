# `sqlgen::col`

`sqlgen::col` provides a type-safe way to reference and manipulate database columns in SQL queries. It enables building complex SQL conditions, ordering, and updates through a fluent C++ interface.

## Usage

### Basic Definition

Reference a column using the string literal operator:

```cpp
using namespace sqlgen;

// Using string literal operator
const auto age_col = "age"_c;

// Using col template function
const auto name_col = col<"first_name">;
```

### Column Operations

#### Comparison Operations

Compare columns with values or other columns:

```cpp
using namespace sqlgen;

// Compare with value
const auto query1 = read<std::vector<Person>> | where("age"_c > 18);

// Compare with another column
const auto query2 = read<std::vector<Person>> | 
                   where("age"_c > "id"_c);
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "age" > 18;

-- For query2
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "age" > "id";
```

#### NULL Operations

Check for NULL or NOT NULL values:

```cpp
using namespace sqlgen;

// Find records where age is NULL
const auto query1 = read<std::vector<Person>> | 
                   where("age"_c.is_null());

// Find records where age is NOT NULL
const auto query2 = read<std::vector<Person>> | 
                   where("age"_c.is_not_null());
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "age" IS NULL;

-- For query2
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "age" IS NOT NULL;
```

#### Pattern Matching

Use LIKE and NOT LIKE for pattern matching:

```cpp
using namespace sqlgen;

// Find names starting with 'H'
const auto query1 = read<std::vector<Person>> | 
                   where("first_name"_c.like("H%"));

// Find names not starting with 'H'
const auto query2 = read<std::vector<Person>> | 
                   where("first_name"_c.not_like("H%"));
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "first_name" LIKE 'H%';

-- For query2
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "first_name" NOT LIKE 'H%';
```

#### Ordering

Specify column ordering in queries:

```cpp
using namespace sqlgen;

// Order by age ascending
const auto query1 = read<std::vector<Person>> | 
                    order_by("age"_c);

// Order by age descending
const auto query2 = read<std::vector<Person>> | 
                    order_by("age"_c.desc());

// Multiple columns with mixed ordering
const auto query3 = read<std::vector<Person>> | 
                    order_by("last_name"_c, "first_name"_c.desc());
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
ORDER BY "age";

-- For query2
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
ORDER BY "age" DESC;

-- For query3
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
ORDER BY "last_name", "first_name" DESC;
```

#### Updates

Set column values in UPDATE statements:

```cpp
using namespace sqlgen;

// Update a single column
const auto query1 = update<Person>("age"_c.set(46));

// Update multiple columns
const auto query2 = update<Person>(
    "first_name"_c.set("last_name"_c),
    "age"_c.set(100)
) | where("first_name"_c == "Hugo");
```

This generates SQL like:

```sql
-- For query1
UPDATE "Person" SET "age" = 46;

-- For query2
UPDATE "Person" 
SET "first_name" = "last_name", "age" = 100 
WHERE "first_name" = 'Hugo';
```

## Type Safety

`sqlgen::col` class provides compile-time type safety:

- Column names are validated at compile time using string literals
- SQL injection is prevented through proper escaping and parameterization

## Notes

- The class supports all standard SQL comparison operators: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Column names are automatically quoted in generated SQL
- The class is designed to be used with the query builder interface
- All operations are composable and can be chained together
- The class supports both value and column-to-column comparisons
- String literals are automatically converted to the appropriate SQL type
- The class is thread-safe and has no mutable state 