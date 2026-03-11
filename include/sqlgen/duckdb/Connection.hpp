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
#include "../Session.hpp"
#include "../Transaction.hpp"
#include "../dynamic/Operation.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "../dynamic/Write.hpp"
#include "../internal/iterator_t.hpp"
#include "../internal/remove_auto_incr_primary_t.hpp"
#include "../internal/strings/strings.hpp"
#include "../internal/to_container.hpp"
#include "../internal/to_str_vec.hpp"
#include "../is_connection.hpp"
#include "../sqlgen_api.hpp"
#include "./parsing/Parser_default.hpp"
#include "DuckDBAppender.hpp"
#include "DuckDBConnection.hpp"
#include "DuckDBResult.hpp"
#include "Iterator.hpp"
#include "to_sql.hpp"

namespace sqlgen::duckdb {

class SQLGEN_API Connection {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  static constexpr bool supports_returning_ids = true;
  static constexpr bool supports_multirow_returning_ids = true;

  Connection(const ConnPtr& _conn) : appender_(nullptr), conn_(_conn) {}

  static rfl::Result<Ref<Connection>> make(
      const std::optional<std::string>& _fname) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string& _sql) noexcept;

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert(const dynamic::Insert& _insert_stmt, ItBegin _begin,
                         ItEnd _end,
                         std::vector<std::optional<std::string>>*
                             _returned_ids = nullptr) noexcept {
    if (_returned_ids) {
      return insert_with_returning(_insert_stmt, _begin, _end, _returned_ids);
    }

    using namespace std::ranges::views;

    const auto sql = to_sql(_insert_stmt);

    auto columns = internal::collect::vector(
        _insert_stmt.columns |
        transform([](const auto& _str) { return _str.c_str(); }));

    return get_duckdb_logical_types(_insert_stmt.table, _insert_stmt.columns)
        .and_then([&](const auto& _types) {
          return DuckDBAppender::make(sql, conn_, columns, _types);
        })
        .and_then([&](auto _appender) {
          return write_to_appender(_begin, _end, _appender->appender())
              .and_then([&](const auto&) { return _appender->close(); });
        });
  }

  template <class ContainerType>
  auto read(const rfl::Variant<dynamic::SelectFrom, dynamic::Union>& _query) {
    using ValueType = transpilation::value_t<ContainerType>;
    const auto sql = _query.visit([&](const auto& _q) { return to_sql(_q); });
    return internal::to_container<ContainerType, Iterator<ValueType>>(
        Iterator<ValueType>(sql, conn_));
  }

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return duckdb::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _write_stmt) {
    if (appender_) {
      return error(
          "Write operation already in progress - you cannot start another.");
    }

    using namespace std::ranges::views;

    auto columns = internal::collect::vector(
        _write_stmt.columns |
        transform([](const auto& _str) { return _str.c_str(); }));

    const auto sql = to_sql(_write_stmt);

    return get_duckdb_logical_types(_write_stmt.table, _write_stmt.columns)
        .and_then([&](auto _types) {
          return DuckDBAppender::make(sql, conn_, columns, _types);
        })
        .transform([&](auto&& _appender) {
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
  std::string build_batched_insert_sql(const dynamic::Insert& _stmt,
                                       size_t _num_rows) {
    using namespace std::ranges::views;

    const auto wrap_in_quotes = [](const auto& _name) {
      return "\"" + _name + "\"";
    };

    std::stringstream stream;

    stream << "INSERT ";

    if (_stmt.conflict_policy == dynamic::Insert::ConflictPolicy::replace) {
      stream << "OR REPLACE ";
    } else if (_stmt.conflict_policy ==
               dynamic::Insert::ConflictPolicy::ignore) {
      stream << "OR IGNORE ";
    }

    stream << "INTO ";

    if (_stmt.table.schema) {
      stream << wrap_in_quotes(*_stmt.table.schema) << ".";
    }

    stream << wrap_in_quotes(_stmt.table.name);

    stream << " (";
    stream << internal::strings::join(
        ", ",
        internal::collect::vector(_stmt.columns | transform(wrap_in_quotes)));
    stream << ") VALUES ";

    // Build single row placeholder: (?, ?, ...)
    std::string row_placeholder = "(";
    for (size_t i = 0; i < _stmt.columns.size(); ++i) {
      if (i > 0) row_placeholder += ", ";
      row_placeholder += "?";
    }
    row_placeholder += ")";

    // Repeat for _num_rows
    for (size_t r = 0; r < _num_rows; ++r) {
      if (r > 0) stream << ", ";
      stream << row_placeholder;
    }

    if (_stmt.returning.size() != 0) {
      stream << " RETURNING ";
      stream << internal::strings::join(
          ", ", internal::collect::vector(_stmt.returning |
                                          transform(wrap_in_quotes)));
    }

    stream << ";";

    return stream.str();
  }

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert_with_returning(
      const dynamic::Insert& _insert_stmt, ItBegin _begin, ItEnd _end,
      std::vector<std::optional<std::string>>* _returned_ids) {
    if (_insert_stmt.returning.size() != 1) {
      return error(
          "DuckDB returning(ids) requires exactly one returned "
          "auto-incrementing primary key column.");
    }

    constexpr size_t BATCH_SIZE = 1000;
    const size_t num_cols = _insert_stmt.columns.size();

    auto it = _begin;
    while (it != _end) {
      // 1. Collect current batch of rows
      std::vector<std::vector<std::optional<std::string>>> batch;
      batch.reserve(BATCH_SIZE);

      for (size_t i = 0; i < BATCH_SIZE && it != _end; ++i, ++it) {
        auto row = internal::to_str_vec(*it);
        if (row.size() != num_cols) {
          return error("Expected " + std::to_string(num_cols) +
                       " values, got " + std::to_string(row.size()) + ".");
        }
        batch.push_back(std::move(row));
      }

      if (batch.empty()) {
        break;
      }

      // 2. Build batched INSERT SQL
      const auto sql = build_batched_insert_sql(_insert_stmt, batch.size());

      // 3. Prepare statement
      duckdb_prepared_statement stmt = nullptr;

      if (duckdb_prepare(conn_->conn(), sql.c_str(), &stmt) == DuckDBError) {
        const auto* raw_err = stmt ? duckdb_prepare_error(stmt) : nullptr;
        const auto err =
            raw_err ? std::string(raw_err)
                    : std::string("Failed to prepare INSERT statement.");
        if (stmt) {
          duckdb_destroy_prepare(&stmt);
        }
        return error(err);
      }

      // 4. Bind all parameters (batch.size() * num_cols)
      idx_t param_idx = 1;
      for (const auto& row : batch) {
        for (const auto& val : row) {
          const auto state =
              val ? duckdb_bind_varchar(stmt, param_idx, val->c_str())
                  : duckdb_bind_null(stmt, param_idx);

          if (state == DuckDBError) {
            const auto* raw_err = duckdb_prepare_error(stmt);
            const auto err = raw_err ? std::string(raw_err)
                                     : std::string("Failed to bind parameter.");
            duckdb_destroy_prepare(&stmt);
            return error(err);
          }
          ++param_idx;
        }
      }

      // 5. Execute once
      duckdb_result result{};
      const auto execute_state = duckdb_execute_prepared(stmt, &result);

      if (execute_state == DuckDBError) {
        const auto* raw_err = duckdb_result_error(&result);
        const auto* stmt_err = duckdb_prepare_error(stmt);
        const auto err =
            raw_err ? std::string(raw_err)
                    : (stmt_err
                           ? std::string(stmt_err)
                           : std::string("Failed to execute prepared INSERT."));
        duckdb_destroy_result(&result);
        duckdb_destroy_prepare(&stmt);
        return error(err);
      }

      // 6. Read all returned IDs
      const idx_t row_count = duckdb_row_count(&result);

      if (row_count != batch.size() || duckdb_column_count(&result) < 1) {
        duckdb_destroy_result(&result);
        duckdb_destroy_prepare(&stmt);
        return error(
            "INSERT ... RETURNING must return exactly one row per input row "
            "and at least one column. Expected " +
            std::to_string(batch.size()) + " rows, got " +
            std::to_string(row_count) + ".");
      }

      for (idx_t r = 0; r < row_count; ++r) {
        if (duckdb_value_is_null(&result, 0, r)) {
          _returned_ids->emplace_back(std::nullopt);
        } else {
          char* raw_value = duckdb_value_varchar(&result, 0, r);

          if (!raw_value) {
            duckdb_destroy_result(&result);
            duckdb_destroy_prepare(&stmt);
            return error("Failed to read returned id from DuckDB.");
          }

          _returned_ids->emplace_back(raw_value);
          duckdb_free(raw_value);
        }
      }

      duckdb_destroy_result(&result);
      duckdb_destroy_prepare(&stmt);
    }

    return Nothing{};
  }

  Result<std::vector<duckdb_logical_type>> get_duckdb_logical_types(
      const dynamic::Table& _table, const std::vector<std::string>& _columns) {
    using namespace std::ranges::views;

    const auto fields = internal::collect::vector(
        _columns | transform([](const auto& _name) {
          return dynamic::SelectFrom::Field{
              .val = dynamic::Operation{dynamic::Column{.alias = std::nullopt,
                                                        .name = _name}},
              .as = std::nullopt};
        }));

    const auto select_from = dynamic::SelectFrom{.table_or_query = _table,
                                                 .fields = fields,
                                                 .limit = dynamic::Limit{0},
                                                 .offset = dynamic::Offset{0}};

    return DuckDBResult::make(to_sql(select_from), conn_)
        .transform([&](const auto& _res) {
          return internal::collect::vector(
              iota(static_cast<idx_t>(0), static_cast<idx_t>(fields.size())) |
              transform(
                  std::bind_front(duckdb_column_logical_type, &_res->res())));
        });
  }

  template <class ItBegin, class ItEnd>
  Result<Nothing> write_to_appender(ItBegin _begin, ItEnd _end,
                                    duckdb_appender _appender) {
    for (auto it = _begin; it != _end; ++it) {
      const auto res = write_row(*it, _appender);
      if (!res) {
        return res;
      }
      const auto state = duckdb_appender_end_row(_appender);
      if (state == DuckDBError) {
        return error(duckdb_appender_error(_appender));
      }
    }
    return Nothing{};
  }

  template <class StructT>
  Result<Nothing> write_row(const StructT& _struct,
                            duckdb_appender _appender) noexcept {
    using ViewType =
        internal::remove_auto_incr_primary_t<rfl::view_t<const StructT>>;
    try {
      ViewType(rfl::to_view(_struct)).apply([&](const auto& _field) {
        using ValueType = std::remove_cvref_t<std::remove_pointer_t<
            typename std::remove_cvref_t<decltype(_field)>::Type>>;
        duckdb::parsing::Parser<ValueType>::write(*_field.value(), _appender)
            .value();
      });
    } catch (const std::exception& e) {
      return error(e.what());
    }
    return Nothing{};
  }

 private:
  /// The appender to be used for the write statements
  std::shared_ptr<DuckDBAppender> appender_;

  /// The underlying duckdb3 connection.
  ConnPtr conn_;
};

}  // namespace sqlgen::duckdb

namespace sqlgen::internal {
template <class ValueType>
struct IteratorType<ValueType, duckdb::Connection> {
  using Type = duckdb::Iterator<ValueType>;
};

static_assert(is_connection<duckdb::Connection>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Session<duckdb::Connection>>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Transaction<duckdb::Connection>>,
              "Must fulfill the is_connection concept.");

}  // namespace sqlgen::internal

#endif
