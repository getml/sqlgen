#ifndef SQLGEN_DUCKDB_CONNECTION_HPP_
#define SQLGEN_DUCKDB_CONNECTION_HPP_

#include <duckdb.h>

#include <memory>
#include <optional>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../Transaction.hpp"
#include "../dynamic/Write.hpp"
#include "../is_connection.hpp"
#include "DuckDBConnection.hpp"
// #include "to_sql.hpp"

namespace sqlgen::duckdb {

class Connection {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  Connection(const ConnPtr& _conn) : /*stmt_(nullptr),*/ conn_(_conn) {}

  static rfl::Result<Ref<Connection>> make(
      const std::optional<std::string>& _fname) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string& _sql) noexcept;

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>&
          _data) noexcept;

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query);

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return "TODO";  // duckdb::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt);

  Result<Nothing> end_write();

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data);

 private:
  /// A prepared statement - needed for the read and write operations. Note that
  /// we have declared it before conn_, meaning it will be destroyed first.
  // StmtPtr stmt_;

  /// The underlying duckdb3 connection.
  ConnPtr conn_;
};

static_assert(is_connection<Connection>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Transaction<Connection>>,
              "Must fulfill the is_connection concept.");

}  // namespace sqlgen::duckdb

#endif
