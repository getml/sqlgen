# sqlgen Documentation

Welcome to the sqlgen documentation. This guide provides detailed information about sqlgen's features and APIs.

## Core Concepts

- [Defining Tables](defining_tables.md) - How to define tables using C++ structs
- [sqlgen::Result](result.md) - How sqlgen handles errors and results
- [sqlgen::PrimaryKey](primary_key.md) - How to define primary keys in sqlgen
- [sqlgen::Flatten](flatten.md) - How to "inherit" fields from other structs
- [sqlgen::to_sql](to_sql.md) - How to transpile C++ operations to dialect-specific SQL

## Database Operations

- [sqlgen::read](reading.md) - How to read data from a database
- [sqlgen::write](writing.md) - How to write data to a database

- [sqlgen::create_index](create_index.md) - How to create an index on a table
- [sqlgen::create_table](create_table.md) - How to create a new table
- [sqlgen::delete_from](delete_from.md) - How to delete data from a table
- [sqlgen::drop](drop.md) - How to drop a table
- [sqlgen::exec](exec.md) - How to execute raw SQL statements
- [sqlgen::insert](insert.md) - How to insert data within transactions
- [sqlgen::update](update.md) - How to update data in a table

- [Transactions](transactions.md) - How to use transactions for atomic operations

## Data Types and Validation

- [sqlgen::Timestamp](timestamp.md) - How timestamps work in sqlgen
- [sqlgen::Varchar](varchar.md) - How varchars work in sqlgen
- [sqlgen::Pattern](pattern.md) - How to add regex pattern validation to avoid SQL injection

## Supported Databases

- [PostgreSQL](postgres.md) - How to interact with PostgreSQL and compatible databases (Redshift, Aurora, Greenplum)
- [SQLite](sqlite.md) - How to interact with SQLite3

For installation instructions, quick start guide, and usage examples, please refer to the [main README](../README.md). 
