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
  - LISTEN/NOTIFY for real-time event notifications

## LISTEN/NOTIFY

PostgreSQL provides a simple publish-subscribe mechanism through `LISTEN` and `NOTIFY` commands. This allows database clients to receive real-time notifications when events occur, without polling. Any client can send a notification to a channel, and all clients listening on that channel will receive it asynchronously.

> **Note:** You should use a dedicated connection for LISTEN/NOTIFY, separate from your main database activity and outside any connection pool. This is because the listening connection must remain open and persistent to receive notifications, and connection pools typically recycle connections which would lose the LISTEN state.

### API

The `Connection` class provides the following methods for listen/notify:

```cpp
// Subscribe to a notification channel
rfl::Result<Nothing> listen(const std::string& channel) noexcept;

// Unsubscribe from a notification channel
rfl::Result<Nothing> unlisten(const std::string& channel) noexcept;

// Send a notification to a channel with an optional payload
rfl::Result<Nothing> notify(const std::string& channel,
                            const std::string& payload = "") noexcept;

// Consume input from the server (must be called before get_notifications)
bool consume_input() noexcept;

// Retrieve all pending notifications
std::list<Notification> get_notifications() noexcept;
```

The `Notification` struct contains:

```cpp
struct Notification {
    std::string channel;   // The channel name
    std::string payload;   // The notification payload (may be empty)
    int backend_pid;       // The PID of the notifying backend
};
```

### Subscribing to Channels

```cpp
auto conn = sqlgen::postgres::connect(creds);
if (!conn) {
    // Handle error...
    return;
}

// Subscribe to a channel
auto result = (*conn)->listen("my_channel");
if (!result) {
    // Handle error...
}
```

### Receiving Notifications

To receive notifications, you must periodically call `consume_input()` to read data from the server, then `get_notifications()` to retrieve any pending notifications:

```cpp
while (running) {
    // Consume any available input from the server
    if (!(*conn)->consume_input()) {
        // Connection error
        break;
    }

    // Process any pending notifications
    auto notifications = (*conn)->get_notifications();
    for (const auto& notification : notifications) {
        // Handle the notification
        std::cout << "Channel: " << notification.channel
                  << " Payload: " << notification.payload << std::endl;
    }

    // Sleep briefly before checking again
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Sending Notifications

```cpp
// Send a notification with a payload
auto result = (*conn)->notify("my_channel", "event data here");
if (!result) {
    // Handle error...
}
```

