# `sqlgen::Timestamp`

The `sqlgen::Timestamp` class provides a type-safe way to handle SQL timestamp fields in C++. It supports both regular timestamps and timestamps with time zones, with format validation at runtime. The class is essentially a wrapper around `std::tm` that adds format validation and SQL integration.

## Usage

### Basic Definition

Define a timestamp field in your struct with a specific format:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S"> birthdate;  // Regular timestamp
    sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S%z"> created_at;  // Timestamp with timezone
};
```

This generates the following SQL schema:

```sql
CREATE TABLE IF NOT EXISTS "Person"(
    "id" INTEGER NOT NULL,
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "birthdate" TIMESTAMP NOT NULL,
    "created_at" TIMESTAMP WITH TIME ZONE NOT NULL,
    PRIMARY KEY ("id")
);
```

### Construction and Assignment

Create and assign timestamp values:

```cpp
// Direct string assignment
Person person{
    .id = 1,
    .first_name = "Homer",
    .last_name = "Simpson",
    .birthdate = "1989-12-17 12:00:00",
    .created_at = "2024-03-20 15:30:00+0000"
};

// Create from std::tm
std::tm birth_tm = std::tm{};
birth_tm.tm_year = 89;  // Years since 1900
birth_tm.tm_mon = 11;   // 0-11 for months
birth_tm.tm_mday = 17;  // Day of month
birth_tm.tm_hour = 12;
birth_tm.tm_min = 0;
birth_tm.tm_sec = 0;
sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S"> birthdate(birth_tm);

// Safe construction with error handling
auto result = sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S">::from_string("1989-12-17 12:00:00");
if (result) {
    auto birthdate = result.value();
    // Use birthdate...
} else {
    // Handle error...
}
```

### Accessing Values

Access the timestamp value in different formats:

```cpp
const auto person = Person{
    .birthdate = "1989-12-17 12:00:00"
};

// Get as formatted string
const std::string& str_value2 = person.birthdate.str(); // Returns "1989-12-17 12:00:00"

// Get as std::tm for time operations
const std::tm& tm_value = person.birthdate.tm();  // Direct access to underlying std::tm
```

### Time Zone Support

The class supports both regular timestamps and timestamps with time zones:

```cpp
// Regular timestamp (without timezone)
sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S"> local_time;

// Timestamp with timezone
sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S%z"> utc_time;

// Examples
local_time = "2024-03-20 15:30:00";  // Local time
utc_time = "2024-03-20 15:30:00+0000";  // UTC time with offset
```

## Format Specification

The timestamp format is specified using a template parameter that follows the strftime format specifiers. This format is significant because:

1. It defines the expected string format when reading timestamps from the database
2. It determines how timestamps are formatted when writing to the database
3. It enforces consistency between your C++ code and database storage

For example, with this definition:
```cpp
struct Person {
    sqlgen::Timestamp<"%Y-%m-%d %H:%M:%S"> birthdate;
};
```

- When reading from the database, the timestamp must be in the format "YYYY-MM-DD HH:MM:SS"
- When writing to the database, the timestamp will be formatted as "YYYY-MM-DD HH:MM:SS"
- Any attempt to assign a string in a different format will throw a runtime error

The most importand supported format specifiers are:

- `%Y`: Year with century (e.g., 2024)
- `%m`: Month (01-12)
- `%d`: Day of month (01-31)
- `%H`: Hour in 24h format (00-23)
- `%M`: Minute (00-59)
- `%S`: Second (00-59)
- `%z`: Timezone offset (e.g., +0000, -0500)

Please refer to https://en.cppreference.com/w/cpp/chrono/c/strftime for a comprehensive list of supported format specifiers.

Common format patterns:
- `"%Y-%m-%d %H:%M:%S"` for regular timestamps
- `"%Y-%m-%d %H:%M:%S%z"` for timestamps with timezone

### Database Integration

The format parameter ensures type safety and consistency in database operations:

```cpp
// Reading from database
const auto people = sqlgen::read<std::vector<Person>>(conn).value();
// All timestamps in 'people' will be in the format "%Y-%m-%d %H:%M:%S"

// Writing to database
const auto person = Person{
    .birthdate = "1989-12-17 12:00:00"  // Must match the format
};
sqlgen::write(conn, person);  // Will be stored in the database in this format

// Querying with timestamps
const auto query = sqlgen::read<std::vector<Person>> | 
                  where("birthdate"_c == "1989-12-17 12:00:00");  // Must match the format
```

This format validation helps prevent:
- Data corruption from inconsistent timestamp formats
- SQL injection by validating the format before database operations
- Runtime errors from malformed timestamp strings

## Notes

- The template parameter `_format` specifies the expected timestamp format
- Format validation occurs at runtime when values are assigned
- The class is a wrapper around `std::tm` that adds:
  - Format validation
  - String conversion
  - SQL integration
  - Type safety
- The class supports:
  - Direct string assignment with format validation
  - Construction from `std::tm`
  - Safe construction with error handling via `from_string`
  - Multiple access methods for the underlying value
  - Direct access to the underlying `std::tm` via `tm()`
  - Reflection for SQL operations
  - Move and copy semantics
- Timezone support varies by database:
  - PostgreSQL supports both regular timestamps and timestamps with timezone
  - SQLite treats all timestamps as UTC
- The class uses the underlying `rfl::Timestamp` implementation for core functionality
- All string operations are validated against the specified format
- Invalid format strings will throw a runtime error
- The class provides both compile-time and runtime safety

