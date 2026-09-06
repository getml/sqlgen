# `sqlgen::Result`

The `sqlgen::Result` type provides a way to handle errors without exceptions in sqlgen. It is an alias for `rfl::Result` and is used throughout sqlgen's API to handle potential failures in database operations.

Note that `sqlgen::Result` and `rfl::Result` are deliberately designed
to resemble C++ 23's [std::expected](https://en.cppreference.com/w/cpp/utility/expected).

## Usage

### Basic Usage

Handle database operations that might fail:

```cpp
// Reading from database
const auto people = sqlgen::read<std::vector<Person>>(conn);

if (people) {
    // Success case - use the value
    const auto& result = *people;
} else {
    // Error case - handle the error
    const auto& error = people.error();
}

// Writing to database
const auto write_result = sqlgen::write(conn, data);

if (!write_result) {
    // Handle write error
    const auto& error = write_result.error();
}
```

### Monadic Operations

The `Result` type supports several monadic operations for chaining operations:

```cpp
// Transform values (T -> U)
const auto count = result.transform([](const auto& people) {
    return people.size();
});

// Chain operations that might fail (T -> Result<U>)
const auto processed = result.and_then([](const auto& people) {
    return process_people(people);  // Returns Result<ProcessedData>
});

// Handle errors with alternative computation (Error -> Result<T>)
const auto recovered = result.or_else([](const auto& error) {
    return fallback_operation();  // Returns Result<T>
});
```

### Value Access Methods

Multiple ways to safely access the contained value:

```cpp
const auto result = sqlgen::read<std::vector<Person>>(conn);

// Method 1: Using .value() (throws if error)
try {
    const auto& people = result.value();
} catch (const std::runtime_error& e) {
    // Handle error
}

// Method 2: Using .value_or() (provides default if error)
const auto people = result.value_or(std::vector<Person>());

// Method 3: Using operator * (undefined if error)
if (result) {
    const auto& people = *result;
}

// Method 4: Using operator -> for member access
if (result) {
    const auto size = result->size();
}

// Method 5: Using .has_value() to check, same as if (result) {...}
if (result.has_value()) {
    // Safe to access value
}
```

### Error Access Methods

Multiple ways to access error information:

```cpp
const auto result = sqlgen::read<std::vector<Person>>(conn);

// Method 1: Using .error() (throws if success)
try {
    const auto& err = result.error();
} catch (const std::runtime_error& e) {
    // Handle case where result contains a value
}

// Method 2: Using .error_or() to provide default error
const auto err = result.error_or(sqlgen::error("No error occurred"));

// Method 3: Using boolean check
if (!result) {
    const auto& err = result.error();
}
```

### Error Creation and Handling

Create and handle errors using the `error` helper function:

```cpp
// Create an error
const auto err = sqlgen::error("Database connection failed");

// Use in Result
const auto result = sqlgen::Result<std::vector<Person>>(sqlgen::error("Failed to read data"));

// Transform errors to provide more context
const auto better_error = result.transform_error([](const auto& e) {
    return sqlgen::error("Database operation failed: " + e.what());
});
```

## Implementation Details

- `sqlgen::Result` is implemented as an alias for `rfl::Result`
- The implementation uses a union-like storage to hold either the value or error
- Move and copy semantics are fully supported
- All operations that might fail return a `Result` type
- The implementation is compatible with C++23's `std::expected` when available

## Best Practices

1. Prefer explicit error handling over using `.value()` which throws exceptions
2. Use monadic operations (`transform`, `and_then`, `or_else`) for clean operation chaining
3. Always check for errors before accessing values
4. Use `value_or()` when a sensible default exists
5. Consider using `transform_error()` to add context to errors
6. Use `error_or()` when you need to provide a default error message

## Notes

- `sqlgen::Result` is an alias for `rfl::Result`
- All database operations in sqlgen return `Result` types
- The type supports:
  - Safe value access with `.value()` and `.value_or()`
  - Monadic operations with `.transform()` and `.and_then()`
  - Error handling with `.error()`
  - Boolean checks for success/failure
  - Move and copy semantics
- It's recommended to handle errors explicitly rather than using `.value()` which throws exceptions
