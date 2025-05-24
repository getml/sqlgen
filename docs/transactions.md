# Transactions

sqlgen provides a simple and safe way to handle database transactions. Transactions ensure that a series of database operations are executed atomically - either all operations succeed, or none of them do.

## Basic Usage

Transactions in sqlgen are managed through three main functions:

- `sqlgen::begin_transaction` - Starts a new transaction
- `sqlgen::commit` - Commits the current transaction
- `sqlgen::rollback` - Rolls back the current transaction 

Here's a basic example of how to use transactions:

```cpp
using namespace sqlgen;

// Start a transaction and chain operations
// Note that all of these operations return
// the connection if they are successful.
auto conn = sqlite::connect("database.db")
            .and_then(begin_transaction)
            .and_then(delete_from<Person> | where("first_name"_c == "Hugo"))
            .and_then(update<Person>("age"_c.set(46)) | where("first_name"_c == "Homer"))
            .and_then(commit);
```

## Automatic Rollback

sqlgen provides automatic rollback protection through RAII (Resource Acquisition Is Initialization). `begin_transaction` creates a `sqlgen::Transaction` object. If a transaction is not explicitly committed, it will be automatically rolled back when the transaction object goes out of scope. This helps ensure database consistency.

## Error Handling

All transaction operations return a `sqlgen::Result<Ref<Connection>>`, which allows for safe error handling and operation chaining. If any operation in the transaction chain fails, the error will be propagated through the Result type, and the transaction will be rolled back.

Example with error handling:

```cpp
using namespace sqlgen;

auto conn = sqlite::connect("database.db")
            .and_then(begin_transaction)
            .and_then(delete_from<Person> | where("first_name"_c == "Hugo"))
            .and_then(update<Person>("age"_c.set(46)) | where("first_name"_c == "Homer"))
            .and_then(commit);

if (!conn) {
    // Handle error
    std::cerr << "Transaction failed: " << result.error() << std::endl;
    return;
}

// Transaction was successful. conn contains the active connection.
```

## Best Practices

1. Chain operations using `.and_then()` to maintain transaction context
2. Let the automatic rollback handle cleanup in case of errors
3. Check the Result type after transaction operations to handle any errors
4. Keep transactions as short as possible to minimize lock contention
