# `sqlgen::postgres`

The `sqlgen::postgres` module provides a type-safe and efficient interface for interacting with PostgreSQL databases. It implements the core database operations through a connection-based API with support for prepared statements, transactions, and efficient data iteration.

## Usage

### Basic Connection

Create a connection to a PostgreSQL database using credentials:

```cpp
// Create credentials for the database connection
const auto creds = sqlgen::postgres::Credentials{
                        .user = "myuser",
                        .password = "mypassword",
                        .host = "localhost",
                        .dbname = "mydatabase",
                        .port = 5432  // Optional, defaults to 5432
                    };

// Connect to the database
const auto conn = sqlgen::postgres::connect(creds);
```

The type of `conn` is `sqlgen::Result<sqlgen::Ref<sqlgen::postgres::Connection>>`, which is useful for error handling:

```cpp
// Handle connection errors
const auto conn = sqlgen::postgres::connect(creds);
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

- The module provides a type-safe interface for PostgreSQL operations
- All operations return `sqlgen::Result<T>` for error handling
- The COPY operator is used under-the-hood for efficient data insertion
- The iterator interface supports batch processing of results
- SQL generation adapts to PostgreSQL's dialect
- The module supports:
  - Connection management with credentials
  - Transactions
  - Prepared statements
  - Efficient batch operations
  - Type-safe SQL generation
  - Error handling through `Result<T>`
  - Resource management through `Ref<T>`
  - Customizable connection parameters (host, port, database name, etc.)

