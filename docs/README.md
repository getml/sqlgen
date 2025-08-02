# sqlgen Documentation

Welcome to the sqlgen documentation. This guide provides detailed information about sqlgen's features and APIs.

## Core Concepts

- [Defining Tables](defining_tables.md) - How to define tables using C++ structs
- [sqlgen::col](col.md) - How to represent columns in queries
- [sqlgen::literals](literals.md) - How to use column and table alias literals in queries
- [sqlgen::Flatten](flatten.md) - How to "inherit" fields from other structs
- [sqlgen::PrimaryKey](primary_key.md) - How to define primary keys in sqlgen
- [sqlgen::Result](result.md) - How sqlgen handles errors and results
- [sqlgen::to_sql](to_sql.md) - How to transpile C++ operations to dialect-specific SQL

## Database I/O

- [sqlgen::read](reading.md) - How to read data from a database
- [sqlgen::write](writing.md) - How to write data to a database

## Database Operations

- [sqlgen::create_index](create_index.md) - How to create an index on a table
- [sqlgen::create_table](create_table.md) - How to create a new table
- [sqlgen::delete_from](delete_from.md) - How to delete data from a table
- [sqlgen::drop](drop.md) - How to drop a table
- [sqlgen::exec](exec.md) - How to execute raw SQL statements
- [sqlgen::group_by and Aggregations](group_by_and_aggregations.md) - How generate GROUP BY queries and aggregate data
- [sqlgen::inner_join, sqlgen::left_join, sqlgen::right_join, sqlgen::full_join](joins.md) - How to join different tables
- [sqlgen::insert](insert.md) - How to insert data within transactions
- [sqlgen::update](update.md) - How to update data in a table

## Other Operations

- [Mathematical Operations](mathematical_operations.md) - How to use mathematical functions in queries (e.g., abs, ceil, floor, exp, trigonometric functions, round).
- [String Operations](string_operations.md) - How to manipulate and transform strings in queries (e.g., length, lower, upper, trim, replace, concat).
- [Type Conversion Operations](type_conversion_operations.md) - How to convert between types safely in queries (e.g., cast int to double).
- [Null Handling Operations](null_handling_operations.md) - How to handle nullable values and propagate nullability correctly (e.g., with coalesce and nullability rules).
- [Timestamp and Date/Time Functions](timestamp_operations.md) - How to work with timestamps, dates, and times (e.g., extract parts, perform arithmetic, convert formats).

## Data Types and Validation

- [sqlgen::ForeignKey](foreign_key.md) - How to establish referential integrity between tables
- [sqlgen::Pattern](pattern.md) - How to add regex pattern validation to avoid SQL injection
- [sqlgen::Timestamp](timestamp.md) - How timestamps work in sqlgen
- [sqlgen::Varchar](varchar.md) - How varchars work in sqlgen

## Other concepts

- [Connection Pool](connection_pool.md) - How to manage database connections efficiently
- [Transactions](transactions.md) - How to use transactions for atomic operations

## Supported Databases

- [MySQL](mysql.md) - How to interact with MariaDB and MySQL 
- [PostgreSQL](postgres.md) - How to interact with PostgreSQL and compatible databases (Redshift, Aurora, Greenplum, CockroachDB, ...)
- [SQLite](sqlite.md) - How to interact with SQLite3

For installation instructions, quick start guide, and usage examples, please refer to the [main README](../README.md). 
