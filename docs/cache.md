# `sqlgen::cache`

The `sqlgen` library provides a high-performance caching mechanism to reduce database load and improve query performance. The cache is designed to be thread-safe and uses a first-in-first-out (FIFO) eviction policy.

## Usage

### Basic Caching Example

To use the cache, you can wrap a query with `sqlgen::cache`. The first time the query is executed, the result is fetched from the database and stored in the cache. Subsequent executions of the same query will return the cached result.

```cpp
#include <sqlgen.hpp>

// Define a table
struct User {
  std::string name;
  int age;
};

// Create a query
const auto query = sqlgen::read<User> | where("name"_c == "John");

// Wrap the query with the cache
const auto cached_query = sqlgen::cache<100>(query);

// Execute the query
const auto user1 = cached_query(conn).value();
const auto user2 = cached_query(conn).value();

// Also OK 
const auto user3 = conn.and_then(
    cache<100>(sqlgen::read<User> | where("name"_c == "John"))).value();

// The cache size will be 1, because the second and third query were served from the cache.
// auto cache_size = cached_query.cache(conn).size(); // cache_size is 1
```

### How it Works

The cache stores the results of queries in memory, using the generated SQL string as the key. When a cached query is executed, `sqlgen` first checks if a result for the corresponding SQL query exists in the cache. If it does, the cached result is returned immediately. Otherwise, the query is executed against the database, and the result is stored in the cache before being returned.

### Eviction Policy

The cache uses a simple first-in-first-out (FIFO) eviction policy. When the cache reaches its maximum size, the oldest entry is removed to make space for the new one. The maximum size of the cache is specified as a template parameter to `sqlgen::cache`.

To create a cache with a virtually unlimited size, you can specify a `max_size` of `0`:

```cpp
// This cache will grow indefinitely.
const auto cached_query = sqlgen::cache<0>(query);
```

### Thread Safety and Concurrency

The cache is thread-safe and can be accessed from multiple threads concurrently. A `std::shared_mutex` is used to protect the cache from data races.

It is important to understand the cache's behavior under high contention. If multiple threads request the same uncached query at the exact same time, the database query might be executed multiple times in parallel. However, as soon as one thread has started the query and placed a future for its result into the cache, any other threads that subsequently request the same query will not start a new database operation. Instead, they will wait for the result of the already running query. This mechanism prevents a "cache stampede" for all but the initial concurrent requests.

## Notes

- The cache is enabled by wrapping a query with `sqlgen::cache`.
- The cache uses a FIFO eviction policy.
- The maximum size of the cache can be configured.
- The cache is thread-safe.

