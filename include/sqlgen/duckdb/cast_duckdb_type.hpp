#ifndef SQLGEN_DUCKDB_CASTDUCKDBTYPE_HPP_
#define SQLGEN_DUCKDB_CASTDUCKDBTYPE_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <type_traits>

#include "../Ref.hpp"
#include "../Result.hpp"

namespace sqlgen::duckdb {

template <class T, class U>
Ref<std::vector<T>> cast_as_vector(const size_t _size, U* _ptr) {
  auto vec = Ref<std::vector<T>>::make(_size);
  for (size_t i = 0; i < _size; ++i) {
    (*vec)[i] = static_cast<T>(_ptr[i]);
  }
  return vec;
}

template <class T>
Result<Ref<std::vector<T>>> cast_duckdb_type(const duckdb_type _type,
                                             const size_t _size,
                                             void* _raw_ptr) {
  if constexpr (!std::is_floating_point_v<T> && !std::is_integral_v<T>) {
    return error("Could not cast");

  } else {
    if (_type == DUCKDB_TYPE_TINYINT) {
      return cast_as_vector<T>(_size, static_cast<int8_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_UTINYINT) {
      return cast_as_vector<T>(_size, static_cast<uint8_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_SMALLINT) {
      return cast_as_vector<T>(_size, static_cast<int16_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_USMALLINT) {
      return cast_as_vector<T>(_size, static_cast<uint16_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_INTEGER) {
      return cast_as_vector<T>(_size, static_cast<int32_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_UINTEGER) {
      return cast_as_vector<T>(_size, static_cast<uint32_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_BIGINT) {
      return cast_as_vector<T>(_size, static_cast<int64_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_UBIGINT) {
      return cast_as_vector<T>(_size, static_cast<uint64_t*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_FLOAT) {
      return cast_as_vector<T>(_size, static_cast<float*>(_raw_ptr));

    } else if (_type == DUCKDB_TYPE_DOUBLE) {
      return cast_as_vector<T>(_size, static_cast<double*>(_raw_ptr));

    } else {
      return error("Could not cast");
    }
  }
}

}  // namespace sqlgen::duckdb

#endif
