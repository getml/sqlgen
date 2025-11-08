#ifndef SQLGEN_DUCKDB_CONNECTION_HPP_
#define SQLGEN_DUCKDB_CONNECTION_HPP_

#include <duckdb.h>

#include <iterator>
#include <memory>
#include <optional>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "../Range.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../Transaction.hpp"
#include "../dynamic/Write.hpp"
#include "../internal/iterator_t.hpp"
#include "../internal/to_container.hpp"
#include "../is_connection.hpp"
#include "../transpilation/get_tablename.hpp"
#include "../transpilation/has_reflection_method.hpp"
#include "../transpilation/is_nullable.hpp"
#include "./parsing/Parser_default.hpp"
#include "DuckDBConnection.hpp"
#include "Iterator.hpp"
#include "parsing/Parser.hpp"
#include "to_sql.hpp"

namespace sqlgen::duckdb {

class Connection {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  Connection(const ConnPtr& _conn) : conn_(_conn) {}

  static rfl::Result<Ref<Connection>> make(
      const std::optional<std::string>& _fname) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string& _sql) noexcept;

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert(const dynamic::Insert&, ItBegin _begin,
                         ItEnd _end) noexcept {
    return error("TODO");
  }

  template <class ContainerType>
  Result<ContainerType> read(const dynamic::SelectFrom& _query) {
    using ValueType = transpilation::value_t<ContainerType>;
    auto res = Ref<duckdb_result>();
    duckdb_query(conn_->conn(), to_sql(_query).c_str(), res.get());
    return internal::to_container<ContainerType, Iterator<ValueType>>(
        Iterator<ValueType>(res, conn_));
  }

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return duckdb::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write&) { return Nothing{}; }

  Result<Nothing> end_write() { return Nothing{}; }

  template <class ItBegin, class ItEnd>
  Result<Nothing> write(ItBegin _begin, ItEnd _end) {
    using T =
        std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;
    const auto tablename = transpilation::get_tablename<T>();
    duckdb_appender appender{};
    if (duckdb_appender_create(conn_->conn(), nullptr, tablename.c_str(),
                               &appender) == DuckDBError) {
      return error("Could not create appender.");
    }
    for (auto it = _begin; it < _end; ++it) {
      const auto res = write_row(*it, appender);
      if (!res) {
        duckdb_appender_destroy(&appender);
        return res;
      }
      duckdb_appender_end_row(appender);
    }
    duckdb_appender_destroy(&appender);
    return Nothing{};
  }

 private:
  template <class StructT>
  Result<Nothing> write_row(const StructT& _struct,
                            duckdb_appender _appender) noexcept {
    Result<Nothing> res = Nothing{};
    rfl::to_view(_struct).apply([&](const auto& _field) {
      using ValueType = std::remove_cvref_t<std::remove_pointer_t<
          typename std::remove_cvref_t<decltype(_field)>::Type>>;
      if (res) {
        res = duckdb::parsing::Parser<ValueType>::write(*_field.value(),
                                                        _appender);
      }
    });
    return res;
  }

 private:
  /// The underlying duckdb3 connection.
  ConnPtr conn_;
};

static_assert(is_connection<Connection>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Transaction<Connection>>,
              "Must fulfill the is_connection concept.");

}  // namespace sqlgen::duckdb

namespace sqlgen::internal {

template <class ValueType>
struct IteratorType<ValueType, duckdb::Connection> {
  using Type = duckdb::Iterator<ValueType>;
};

}  // namespace sqlgen::internal

#endif
