# Other Operations and Functions

The `sqlgen` library provides a rich set of SQL operations and functions that can be used in a type-safe and composable way within C++ queries. These operations cover mathematical, string, type conversion, and null-handling functions, and are designed to closely mirror SQL's expressive power.

## Usage

You can use these functions in your `select_from` queries, often in combination with column expressions, literals, and other operations. All functions are available in the `sqlgen` namespace.

---

## Mathematical Functions

### `abs`
Returns the absolute value of a numeric expression.

```cpp
abs("age"_c * (-1)) | as<"abs_age">
```

### `ceil` / `floor`
Rounds a numeric value up (`ceil`) or down (`floor`) to the nearest integer.

```cpp
ceil("salary"_c) | as<"salary_ceiled">
floor("salary"_c) | as<"salary_floored">
```

### `exp`, `ln`, `log2`, `sqrt`
- `exp(x)`: Exponential function (e^x)
- `ln(x)`: Natural logarithm
- `log2(x)`: Base-2 logarithm
- `sqrt(x)`: Square root

```cpp
round(exp(cast<double>("age"_c)), 2) | as<"exp_age">
round(sqrt(cast<double>("age"_c)), 2) | as<"sqrt_age">
```

### `sin`, `cos`, `tan`
Trigonometric functions.

```cpp
sin("angle"_c) | as<"sin_angle">
cos("angle"_c) | as<"cos_angle">
tan("angle"_c) | as<"tan_angle">
```

### `round`
Rounds a numeric value to a specified number of decimal places.

```cpp
round("price"_c, 2) | as<"rounded_price">
```

---

## String Functions

### `length`
Returns the length of a string.

```cpp
length(trim("first_name"_c)) | as<"length_first_name">
```

### `lower` / `upper`
Converts a string to lowercase or uppercase.

```cpp
lower("first_name"_c) | as<"first_name_lower">
upper("first_name"_c) | as<"first_name_upper">
```

### `ltrim`, `rtrim`, `trim`
Removes whitespace (or a specified character) from the left, right, or both sides of a string.

```cpp
ltrim("first_name"_c) | as<"ltrimmed_name">
rtrim("last_name"_c) | as<"rtrimmed_name">
trim("nickname"_c) | as<"trimmed_nickname">
// With custom characters:
ltrim("field"_c, "_ ") | as<"ltrimmed_field">
```

### `replace`
Replaces all occurrences of a substring with another substring.

```cpp
replace("first_name"_c, "Bart", "Hugo") | as<"first_name_replaced">
```

### `concat`
Concatenates multiple strings or expressions.

```cpp
concat(ltrim("first_name"_c), " ", rtrim("last_name"_c)) | as<"full_name">
concat(upper("last_name"_c), ", ", "first_name"_c) | as<"full_name">
```

---

## Type Conversion

### `cast`
Casts a value to a different type (e.g., int to double).

```cpp
cast<double>("age"_c) | as<"age_as_double">
```

---

## Null Handling

### `coalesce`
Returns the first non-null value in the argument list.

```cpp
coalesce("last_name"_c, "none") | as<"last_name_or_none">
coalesce(upper("last_name"_c), "none") | as<"last_name_or_none">
```

---

## Nullable Values

When using these operations on nullable columns (e.g., `std::optional<T>`), the result will also be nullable if any operand is nullable. For example, adding two `std::optional<int>` columns will yield a `std::optional<int>`. The `coalesce` function is especially useful for providing default values for nullable columns.

---

## Nullability Propagation and `coalesce` Semantics

### General Nullability Rules

- **Unary operations** (e.g., `abs`, `upper`, `sqrt`):
  - If the operand is nullable (`std::optional<T>`), the result is also nullable.
  - If the operand is not nullable, the result is not nullable.
- **Binary operations** (e.g., `+`, `concat`, `replace`, etc.):
  - If *any* operand is nullable, the result is nullable (`std::optional<ResultType>`).
  - If *all* operands are non-nullable, the result is non-nullable.
- **Type conversion (`cast`)**:
  - If the source is nullable, the result is nullable of the target type.
  - If the source is not nullable, the result is not nullable.
- **String operations** (e.g., `concat`, `replace`, `ltrim`, `rtrim`, `trim`):
  - If any input is nullable, the result is nullable.
  - All string operands must have the same underlying type (checked at compile time).

### `coalesce` Nullability Semantics

The `coalesce` function returns the first non-null value from its arguments. Its nullability is determined as follows:

- If **all** arguments are nullable, the result is nullable (`std::optional<T>`).
- If **any** argument is non-nullable, the result is non-nullable (`T`).
- All arguments must have the same underlying type (ignoring nullability), enforced at compile time.

#### Examples

```cpp
// All arguments nullable: result is nullable
coalesce(std::optional<int>{}, std::optional<int>{}) // -> std::optional<int>

// At least one argument non-nullable: result is non-nullable
coalesce(std::optional<int>{}, 42) // -> int
coalesce(42, std::optional<int>{}) // -> int

// All arguments non-nullable: result is non-nullable
coalesce(1, 2) // -> int

// Mixed string example
coalesce(std::optional<std::string>{}, "default") // -> std::string

// Compile-time error: mismatched types
// coalesce(std::optional<int>{}, std::optional<double>{}) // Error
```

#### Practical Usage

```cpp
// Provide a default for a nullable column
coalesce("last_name"_c, "none") | as<"last_name_or_none"> // Result is std::string

// All columns nullable: result is nullable
coalesce("middle_name"_c, "nickname"_c) | as<"any_name"> // Result is std::optional<std::string>
```

### Advanced: How sqlgen Enforces Nullability

The nullability rules are enforced at compile time using template metaprogramming (see `underlying_t.hpp`). This ensures that:
- You cannot accidentally assign a nullable result to a non-nullable field (or vice versa).
- All arguments to `coalesce` must have the same base type (e.g., all `int` or all `std::string`).
- The result type of any operation is always correct and safe to use in your result structs.

---

## Example: Combining Operations

```cpp
struct Children {
    int id_plus_age;
    int age_times_2;
    int id_plus_2_minus_age;
    int abs_age;
    double exp_age;
    double sqrt_age;
    size_t length_first_name;
    std::string full_name;
    std::string first_name_lower;
    std::string first_name_upper;
    std::string first_name_replaced;
};

const auto get_children = select_from<Person>(
    ("id"_c + "age"_c) | as<"id_plus_age">,
    ("age"_c * 2) | as<"age_times_2">,
    abs("age"_c * (-1)) | as<"abs_age">,
    round(exp(cast<double>("age"_c)), 2) | as<"exp_age">,
    round(sqrt(cast<double>("age"_c)), 2) | as<"sqrt_age">,
    length(trim("first_name"_c)) | as<"length_first_name">,
    concat(ltrim("first_name"_c), " ", rtrim("last_name"_c)) | as<"full_name">,
    lower(ltrim(concat(" ", "first_name"_c))) | as<"first_name_lower">,
    upper(rtrim(concat("first_name"_c, " "))) | as<"first_name_upper">,
    replace("first_name"_c, "Bart", "Hugo") | as<"first_name_replaced">
) | where("age"_c < 18) | to<std::vector<Children>>;
```

---

## Notes

- All functions are type-safe and map to the appropriate SQL operations for the target database.
- You can chain and nest operations as needed.
- Use the `as<"alias">(...)` or `| as<"alias">` syntax to alias expressions for mapping to struct fields.

