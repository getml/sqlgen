# `sqlgen::unite` and `sqlgen::unite_all`

`sqlgen::unite` and `sqlgen::unite_all` provide a type-safe way to combine multiple SELECT statements into a single SQL UNION (or UNION ALL) expression and use the combined result as a query or a subquery.

Both functions accept an arbitrary number of select statements (the results of `select_from<...>(...)`) and return a callable object that can be executed against a database connection or composed into larger queries (for example, used in JOINs or as a subquery in another `select_from`).

## Basic usage

- `unite<Container>(s1, s2, ...)` returns an object that, when invoked with a connection, executes the UNION of the provided selects and converts the results into the specified `Container` (e.g., `std::vector<T>`). If `Container` is omitted, the default behavior returns a lazy `Range` of results.
- `unite_all<Container>(...)` behaves the same but emits `UNION ALL` (keeps duplicate rows) instead of `UNION` (removes duplicates).

Example (based on tests):

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct User1 { std::string name; int age; };
struct User2 { std::string name; int age; };
struct User3 { int age; std::string name; };

const auto s1 = select_from<User1>("name"_c, "age"_c);
const auto s2 = select_from<User2>("name"_c, "age"_c);
const auto s3 = select_from<User3>("name"_c, "age"_c);

// Build a UNION, convert to vector<User1> and execute
const auto result = unite<std::vector<User1>>(s1, s2, s3)(conn);
const auto users = result.value();
```

This will generate SQL similar to:

```
SELECT "name", "age" FROM (SELECT "name", "age" FROM "User1")
UNION
SELECT "name", "age" FROM (SELECT "name", "age" FROM "User2")
UNION
SELECT "name", "age" FROM (SELECT "name", "age" FROM "User3")
```

And `unite_all` generates `UNION ALL` between the parts:

```
SELECT "name", "age" FROM (SELECT "name", "age" FROM "User1")
UNION ALL
SELECT "name", "age" FROM (SELECT "name", "age" FROM "User2")
UNION ALL
SELECT "name", "age" FROM (SELECT "name", "age" FROM ""User3")
```

You can obtain the SQL text (for supported backends) using the backend-specific `to_sql` helper, e.g. `sqlite::to_sql(sqlgen::unite<std::vector<User1>>(s1,s2,s3))`.

## Composing with other queries (using as subquery / joining)

The result of `unite` / `unite_all` can be used as a table/subquery in other query builders. For example, using a UNION as the right side of a join:

```cpp
struct Login { int id; std::string username; };

const auto united = unite<std::vector<User1>>(s1, s2, s3);

const auto sel = select_from<Login, "t1">("id"_t1, "username"_t1)
                 | inner_join<"t2">(united, "username"_t1 == "name"_t2)
                 | where("id"_t1 == 1)
                 | to<std::vector<Login>>;

const auto query = sqlite::to_sql(sel);
```

This will produce SQL where the UNION appears as a subquery with an alias (for example `t2`) and can participate in joins and where clauses.

## Result typing and nullability

- If a concrete container type (like `std::vector<T>`) is provided, `unite`/`unite_all` will attempt to convert each row into `T` and return `Result<Container>` when invoked with a connection.
- If `Nothing` (the default when no explicit container is provided) is used, the callable returns a lazy `Range` of optional named-tuples representing each row; this is useful for streaming large results without materializing a container.
- When used inside other queries (e.g., as a table in `select_from<"t1">(...)` or for joins), the compile-time transpilation machinery extracts the field types from the unioned selects to determine the table schema and nullability appropriate for the context.

## Notes and recommendations

- Field order and names must match across all select statements used in the union (the transpiler maps fields by position and/or names when producing the combined result type).
- Prefer `unite` when duplicate rows should be eliminated, and `unite_all` when duplicates must be preserved (semantically matches SQL `UNION` vs `UNION ALL`).
- Use `to<Container>` (e.g., `to<std::vector<T>>`) when you want a concrete container result; omit it to obtain a lazy `Range`.

## Error handling

Executing the returned callable against a connection yields `Result<...>`; inspect the `Result` for errors before accessing values (see `select_from` docs for examples).

## Examples in tests

- `tests/sqlite/test_union.cpp` demonstrates `unite<std::vector<User1>>(s1, s2, s3)` producing the expected SQL and returning the distinct rows.
- `tests/sqlite/test_union_all.cpp` demonstrates `unite_all<std::vector<User1>>(...)` preserving duplicates.
- `tests/sqlite/test_union_in_join.cpp` shows using a `unite` result as the right-hand side of an `inner_join`.
