# `sqlgen::sqlite`

The `sqlgen::sqlite` module provides a type-safe and efficient interface for interacting with SQLite databases. It implements the core database operations through a connection-based API with support for prepared statements, transactions, and efficient data iteration.

## Usage

### Basic Connection

Create a connection to a SQLite database:

```cpp
// Connect to an in-memory database
const auto conn = sqlgen::sqlite::connect();

// Connect to a file-based database
const auto conn = sqlgen::sqlite::connect("database.db");
```

The connection is wrapped in a `sqlgen::Result<Ref<Connection>>` for error handling:

```cpp
// Handle connection errors
const auto conn = sqlgen::sqlite::connect("database.db");
if (!conn) {
    // Handle error...
    return;
}

using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read<std::vector<Person>> |
                   where("age"_c < 18 and "first_name"_c != "Hugo");

// Use the connection
const auto minors = query(conn);
```

## Notes

- The module provides a type-safe interface for SQLite operations
- All operations return `sqlgen::Result<T>` for error handling
- Prepared statements are used for efficient query execution
- The iterator interface supports batch processing of results
- SQL generation adapts to SQLite's dialect
- The module supports:
  - In-memory and file-based databases
  - Transactions
  - Prepared statements
  - Efficient batch operations
  - Type-safe SQL generation
  - Error handling through `Result<T>`
  - Resource management through `Ref<T>`

