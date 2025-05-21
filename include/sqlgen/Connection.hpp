#ifndef SQLGEN_CONNECTION_HPP_
#define SQLGEN_CONNECTION_HPP_

#include <optional>
#include <string>
#include <vector>

#include "IteratorBase.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "dynamic/SelectFrom.hpp"
#include "dynamic/Statement.hpp"
#include "dynamic/Write.hpp"

namespace sqlgen {

/// Abstract base class to be implemented by the different
/// database connections.
struct Connection {
  virtual ~Connection() = default;

  /// Begins a transaction.
  virtual Result<Nothing> begin_transaction() = 0;

  /// Commits a transaction.
  virtual Result<Nothing> commit() = 0;

  /// Executes a statement. Note that in order for the statement to take effect,
  /// you must call .commit() afterwards.
  virtual Result<Nothing> execute(const std::string& _sql) = 0;

  /// Inserts data into the database using the INSERT statement.
  /// More minimal approach than write, but can be used inside transactions.
  virtual Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>& _data) = 0;

  /// Reads the results of a SelectFrom statement.
  virtual Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query) = 0;

  /// Rolls a transaction back.
  virtual Result<Nothing> rollback() = 0;

  /// Transpiles a statement to a particular SQL dialect.
  virtual std::string to_sql(const dynamic::Statement& _stmt) = 0;

  /// Starts the write operation.
  virtual Result<Nothing> start_write(const dynamic::Write& _stmt) = 0;

  /// Ends the write operation and thus commits the results.
  virtual Result<Nothing> end_write() = 0;

  /// Writes data into a table. Each vector in data MUST have the same length as
  /// _stmt.columns.
  /// You MUST call .start_write(...) first and call .end_write() after all
  /// the data has been written.
  /// You CAN write the data in chunks, meaning you can call .write(...) more
  /// than once between .start_write(...) and .end_write().
  virtual Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data) = 0;
};

}  // namespace sqlgen

#endif
