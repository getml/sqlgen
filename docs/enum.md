# `Enums`

Enums can directly be used in sqlgen structs. They are mapped to native enum types in PostgreSQL and MySQL. In sqlite, which does not support native enum types, they are stored as `TEXT` by default.

## Usage

### Basic Definition

```cpp
enum class Color : int { RED = 1, GREEN = 2, BLUE = 3 };
struct Flower {
    sqlgen::PrimaryKey<std::string> name;
    Color color;
}

const auto red_rose = Flower{
    .name = "Rose",
    .color = Color::RED,
};
```

This generates the following SQL schema:
```sql
CREATE TYPE IF NOT EXISTS "Color" AS ENUM ('RED', 'GREEN', 'BLUE');
CREATE TABLE IF NOT EXISTS "Flower"(
    "name"  TEXT NOT NULL,
    "color" Color NOT NULL,
    PRIMARY_KEY("name")
);
```

## Notes
- Due to naming restrictions in PostgreSQL, the namespace operator `::` is replaced with `__`. Thus, an enum defined as `namespace1::some_struct::MyEnum` will be created in the database as `namespace1__some_struct__MyEnum`. This mangled name can at most be 63 characters long.
- Enums are specific types in PostgreSQL. They are only created once and shared between tables.
- In MySQL, enums are stored as native types but they are not shared between tables.
- In sqlite, enums are stored as `TEXT` by default. If you need to use integers, you can specialize the `Parser<T>` struct as explained [here](dynamic.md).
- You can use `rfl::enum_to_string` and `rfl::string_to_enum` to convert between enum values and their string representations.