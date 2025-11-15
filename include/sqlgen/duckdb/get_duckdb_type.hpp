#ifndef SQLGEN_DUCKDB_GETDUCKDBTYPE_HPP_
#define SQLGEN_DUCKDB_GETDUCKDBTYPE_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <type_traits>

#include "../Result.hpp"

namespace sqlgen::duckdb {

template <class T>
duckdb_type get_duckdb_type() {
  using Type = std::remove_cvref_t<T>;

  if constexpr (std::is_same_v<Type, bool>) {
    return DUCKDB_TYPE_BOOLEAN;

  } else if constexpr (std::is_same_v<Type, char> ||
                       std::is_same_v<Type, int8_t>) {
    return DUCKDB_TYPE_TINYINT;

  } else if constexpr (std::is_same_v<Type, uint8_t>) {
    return DUCKDB_TYPE_UTINYINT;

  } else if constexpr (std::is_same_v<Type, int16_t>) {
    return DUCKDB_TYPE_SMALLINT;

  } else if constexpr (std::is_same_v<Type, uint16_t>) {
    return DUCKDB_TYPE_USMALLINT;

  } else if constexpr (std::is_same_v<Type, int32_t>) {
    return DUCKDB_TYPE_INTEGER;

  } else if constexpr (std::is_same_v<Type, uint32_t>) {
    return DUCKDB_TYPE_UINTEGER;

  } else if constexpr (std::is_same_v<Type, int64_t>) {
    return DUCKDB_TYPE_BIGINT;

  } else if constexpr (std::is_same_v<Type, uint64_t>) {
    return DUCKDB_TYPE_UBIGINT;

  } else if constexpr (std::is_same_v<Type, float>) {
    return DUCKDB_TYPE_FLOAT;

  } else if constexpr (std::is_same_v<Type, double>) {
    return DUCKDB_TYPE_DOUBLE;

  } else if constexpr (std::is_same_v<Type, duckdb_date>) {
    return DUCKDB_TYPE_DATE;

  } else if constexpr (std::is_same_v<Type, duckdb_string_t>) {
    return DUCKDB_TYPE_VARCHAR;

  } else if constexpr (std::is_same_v<Type, duckdb_timestamp>) {
    return DUCKDB_TYPE_TIMESTAMP;

  } else {
    static_assert(rfl::always_false_v<T>, "Unsupported type.");
  }
}

}  // namespace sqlgen::duckdb

#endif
