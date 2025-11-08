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

#include "../Ref.hpp"
#include "../Result.hpp"
#include "../Transaction.hpp"
#include "../dynamic/Write.hpp"
#include "../internal/iterator_t.hpp"
#include "../is_connection.hpp"
#include "../transpilation/get_tablename.hpp"
#include "../transpilation/has_reflection_method.hpp"
#include "../transpilation/is_nullable.hpp"
#include "DuckDBConnection.hpp"
#include "Iterator.hpp"
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
    return error("TODO");
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
  template <class T>
  Result<Nothing> append_value(const T& _t,
                               duckdb_appender _appender) noexcept {
    using Type = std::remove_cvref_t<T>;

    if constexpr (transpilation::has_reflection_method<Type>) {
      return append_value(_t.reflection(), _appender);

    } else if constexpr (std::is_same_v<Type, bool>) {
      return duckdb_append_bool(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append boolean value."));

    } else if constexpr (std::is_same_v<Type, char> ||
                         std::is_same_v<Type, int8_t>) {
      return duckdb_append_int8(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append int8 value."));

    } else if constexpr (std::is_same_v<Type, uint8_t>) {
      return duckdb_append_uint8(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append uint8 value."));

    } else if constexpr (std::is_same_v<Type, int16_t>) {
      return duckdb_append_int16(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append int16 value."));

    } else if constexpr (std::is_same_v<Type, uint16_t>) {
      return duckdb_append_uint16(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append uint16 value."));

    } else if constexpr (std::is_same_v<Type, int32_t>) {
      return duckdb_append_int32(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append int32 value."));

    } else if constexpr (std::is_same_v<Type, uint32_t>) {
      return duckdb_append_uint32(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append uint32 value."));

    } else if constexpr (std::is_same_v<Type, int64_t>) {
      return duckdb_append_int64(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append int64 value."));

    } else if constexpr (std::is_same_v<Type, uint64_t>) {
      return duckdb_append_uint64(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append uint64 value."));

    } else if constexpr (std::is_same_v<Type, float>) {
      return duckdb_append_float(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append float value."));

    } else if constexpr (std::is_same_v<Type, double>) {
      return duckdb_append_double(_appender, _t) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append double value."));

    } else if constexpr (std::is_same_v<Type, std::string>) {
      return duckdb_append_varchar(_appender, _t.c_str()) != DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append string value."));

    } else if constexpr (std::is_same_v<Type, std::string>) {
      return duckdb_append_varchar_length(_appender, _t.c_str(), _t.length()) !=
                     DuckDBError
                 ? Result<Nothing>(Nothing{})
                 : Result<Nothing>(error("Could not append string value."));

    } else if constexpr (transpilation::is_nullable_v<Type>) {
      if (_t) {
        return append_value(*_t, _appender);
      } else {
        return duckdb_append_null(_appender) != DuckDBError
                   ? Result<Nothing>(Nothing{})
                   : Result<Nothing>(error("Could not append null value."));
      }

    } else {
      static_assert(rfl::always_false_v<T>, "Unsupported type.");
      return error("Unsupported type.");
    }
  }

  template <class StructT>
  Result<Nothing> write_row(const StructT& _struct,
                            duckdb_appender _appender) noexcept {
    Result<Nothing> res = Nothing{};
    rfl::to_view(_struct).apply([&](const auto& _field) {
      if (res) {
        res = append_value(*_field.value(), _appender);
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
