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
#include "sqlgen/dynamic/Operation.hpp"
#include "sqlgen/dynamic/SelectFrom.hpp"
#include "sqlgen/transpilation/get_schema.hpp"
#include "sqlgen/transpilation/to_table_or_query.hpp"
#include "to_sql.hpp"

namespace sqlgen::duckdb {

class Connection {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  Connection(const ConnPtr &_conn) : conn_(_conn) {}

  static rfl::Result<Ref<Connection>> make(
      const std::optional<std::string> &_fname) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string &_sql) noexcept;

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert(const dynamic::Insert &_insert_stmt, ItBegin _begin,
                         ItEnd _end) noexcept {
    const auto sql = to_sql(_insert_stmt);

    return get_duckdb_logical_types(_insert_stmt.table, _insert_stmt.columns)
        .and_then([&](const auto &_types) -> Result<Nothing> {
          duckdb_appender appender{};
          if (duckdb_appender_create_query(
                  conn_->conn(), sql.c_str(),
                  static_cast<idx_t>(_insert_stmt.columns.size()),
                  _types.data(), nullptr, nullptr, &appender) == DuckDBError) {
            return error("Could not create appender.");
          }
          const auto res = write_to_appender(_begin, _end, appender);
          duckdb_appender_destroy(&appender);
          return res;
        });
  }

  template <class ContainerType>
  Result<ContainerType> read(const dynamic::SelectFrom &_query) {
    using ValueType = transpilation::value_t<ContainerType>;
    auto res = Ref<duckdb_result>();
    duckdb_query(conn_->conn(), to_sql(_query).c_str(), res.get());
    return internal::to_container<ContainerType, Iterator<ValueType>>(
        Iterator<ValueType>(res, conn_));
  }

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement &_stmt) noexcept {
    return duckdb::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write &) { return Nothing{}; }

  Result<Nothing> end_write() { return Nothing{}; }

  template <class ItBegin, class ItEnd>
  Result<Nothing> write(ItBegin _begin, ItEnd _end) {
    using T =
        std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;
    const auto schema = transpilation::get_schema<T>();
    const auto table = transpilation::get_tablename<T>();
    duckdb_appender appender{};
    if (duckdb_appender_create(conn_->conn(),
                               schema ? schema->c_str() : nullptr,
                               table.c_str(), &appender) == DuckDBError) {
      return error("Could not create appender.");
    }
    const auto res = write_to_appender(_begin, _end, appender);
    duckdb_appender_destroy(&appender);
    return res;
  }

 private:
  Result<std::vector<duckdb_logical_type>> get_duckdb_logical_types(
      const dynamic::Table &_table, const std::vector<std::string> &_columns) {
    using namespace std::ranges::views;

    const auto fields = internal::collect::vector(
        _columns | transform([](const auto &_name) {
          return dynamic::SelectFrom::Field{
              .val = dynamic::Operation{dynamic::Column{.alias = std::nullopt,
                                                        .name = _name}},
              .as = std::nullopt};
        }));

    const auto select_from = dynamic::SelectFrom{
        .table_or_query = _table, .fields = fields, .limit = dynamic::Limit{0}};

    auto res = Ref<duckdb_result>();

    duckdb_query(conn_->conn(), to_sql(select_from).c_str(), res.get());

    return internal::collect::vector(
        iota(static_cast<idx_t>(fields.size())) |
        transform(std::bind_front(duckdb_column_logical_type, res.get())));
  }

  template <class ItBegin, class ItEnd>
  Result<Nothing> write_to_appender(ItBegin _begin, ItEnd _end,
                                    duckdb_appender _appender) {
    for (auto it = _begin; it < _end; ++it) {
      const auto res = write_row(*it, _appender);
      if (!res) {
        return res;
      }
      duckdb_appender_end_row(_appender);
    }
    return Nothing{};
  }

  template <class StructT>
  Result<Nothing> write_row(const StructT &_struct,
                            duckdb_appender _appender) noexcept {
    Result<Nothing> res = Nothing{};
    rfl::to_view(_struct).apply([&](const auto &_field) {
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
