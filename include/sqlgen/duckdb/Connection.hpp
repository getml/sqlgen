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

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert(const dynamic::Insert& _stmt, ItBegin _begin,
                         ItEnd _end) noexcept;

  template <class ContainerType>
  Result<ContainerType> read(const dynamic::SelectFrom& _query) {
    return error("TODO");
  }

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return "TODO";  // duckdb::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt);

  Result<Nothing> end_write();

  template <class ItBegin, class ItEnd>
  Result<Nothing> write(ItBegin _begin, ItEnd _end) {
    return error("TODO");
  }

 private:
  template <class T>
  Result<Nothing> append_value(const T& _t,
                               duckdb_appender _appender) noexcept {
    using Type = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<Type, bool>) {
      const auto state = duckdb_append_bool(_appender, _t);
      if (state == DuckDBError) {
        return error("Could not append boolean value.");
      }
    }
    return Nothing{};
  }

  template <class StructT>
  Result<Nothing> insert_row(const StructT& _struct,
                             duckdb_appender _appender) noexcept {
    Result<Nothing> res = Nothing{};
    rfl::to_view(_t).apply([&](const auto& _field) {});
    return res;
  }

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
