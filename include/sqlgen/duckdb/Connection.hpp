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
#include "../internal/remove_auto_incr_primary_t.hpp"
#include "../internal/to_container.hpp"
#include "../is_connection.hpp"
#include "./parsing/Parser_default.hpp"
#include "DuckDBAppender.hpp"
#include "DuckDBConnection.hpp"
#include "DuckDBResult.hpp"
#include "Iterator.hpp"
#include "sqlgen/dynamic/Operation.hpp"
#include "sqlgen/dynamic/SelectFrom.hpp"
#include "to_sql.hpp"

namespace sqlgen::duckdb {

class Connection {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  Connection(const ConnPtr &_conn) : appender_(nullptr), conn_(_conn) {}

  static rfl::Result<Ref<Connection>> make(
      const std::optional<std::string> &_fname) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string &_sql) noexcept;

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert(const dynamic::Insert &_insert_stmt, ItBegin _begin,
                         ItEnd _end) noexcept {
    using namespace std::ranges::views;

    const auto sql = to_sql(_insert_stmt);

    auto columns = internal::collect::vector(
        _insert_stmt.columns |
        transform([](const auto &_str) { return _str.c_str(); }));

    return get_duckdb_logical_types(_insert_stmt.table, _insert_stmt.columns)
        .and_then([&](const auto &_types) {
          return DuckDBAppender::make(sql, conn_, columns, _types);
        })
        .and_then([&](const auto &_appender) {
          return write_to_appender(_begin, _end, _appender->appender());
        });
  }

  template <class ContainerType>
  auto read(const dynamic::SelectFrom &_query) {
    using ValueType = transpilation::value_t<ContainerType>;
    return internal::to_container<ContainerType, Iterator<ValueType>>(
        Iterator<ValueType>(to_sql(_query), conn_));
  }

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement &_stmt) noexcept {
    return duckdb::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write &_write_stmt) {
    if (appender_) {
      return error(
          "Write operation already in progress - you cannot start another.");
    }

    using namespace std::ranges::views;

    auto columns = internal::collect::vector(
        _write_stmt.columns |
        transform([](const auto &_str) { return _str.c_str(); }));

    const auto sql = to_sql(_write_stmt);

    return get_duckdb_logical_types(_write_stmt.table, _write_stmt.columns)
        .and_then([&](auto _types) {
          return DuckDBAppender::make(sql, conn_, columns, _types);
        })
        .transform([&](auto &&_appender) {
          appender_ = _appender.ptr();
          return Nothing{};
        });
  }

  Result<Nothing> end_write() {
    if (!appender_) {
      return error("No write operation in progress - nothing to end.");
    }
    appender_ = nullptr;
    return Nothing{};
  }

  template <class ItBegin, class ItEnd>
  Result<Nothing> write(ItBegin _begin, ItEnd _end) {
    if (!appender_) {
      return error("No write operation in progress - nothing to write.");
    }
    return write_to_appender(_begin, _end, appender_->appender());
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

    return DuckDBResult::make(to_sql(select_from), conn_)
        .transform([&](const auto &_res) {
          return internal::collect::vector(
              iota(static_cast<idx_t>(0), static_cast<idx_t>(fields.size())) |
              transform(
                  std::bind_front(duckdb_column_logical_type, &_res->res())));
        });
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
    using ViewType =
        internal::remove_auto_incr_primary_t<rfl::view_t<const StructT>>;
    Result<Nothing> res = Nothing{};
    ViewType(rfl::to_view(_struct)).apply([&](const auto &_field) {
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
  /// The appender to be used for the write statements
  std::shared_ptr<DuckDBAppender> appender_;

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
