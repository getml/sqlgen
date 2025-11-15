#ifndef SQLGEN_DUCKDB_MAKECHUNKPTRS_HPP_
#define SQLGEN_DUCKDB_MAKECHUNKPTRS_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <rfl/enums.hpp>
#include <type_traits>
#include <utility>

#include "../Ref.hpp"
#include "ColumnData.hpp"
#include "DuckDBResult.hpp"
#include "chunk_ptrs_t.hpp"
#include "get_duckdb_type.hpp"

namespace sqlgen::duckdb {

template <class T>
struct MakeChunkPtrs;

template <class... Ts, class... ColNames>
struct MakeChunkPtrs<rfl::Tuple<ColumnData<Ts, ColNames>...>> {
  Result<rfl::Tuple<ColumnData<Ts, ColNames>...>> operator()(
      const Ref<DuckDBResult>& _res, duckdb_data_chunk _chunk) {
    try {
      return [&]<int... _is>(std::integer_sequence<int, _is...>) {
        return rfl::Tuple<ColumnData<Ts, ColNames>...>(
            make_column_data<Ts, ColNames, _is>(_res, _chunk)...);
      }(std::make_integer_sequence<int, sizeof...(Ts)>());
    } catch (const std::exception& e) {
      return error(e.what());
    }
  }

  template <class T, class ColName, int _i>
  static auto make_column_data(const Ref<DuckDBResult>& _res,
                               duckdb_data_chunk _chunk) {
    if (duckdb_column_type(&_res->res(), _i) != get_duckdb_type<T>()) {
      throw std::runtime_error(
          "Wrong type in field " + std::to_string(_i) + ". Expected " +
          rfl::enum_to_string(get_duckdb_type<T>()) + ", got " +
          rfl::enum_to_string(duckdb_column_type(&_res->res(), _i)) + ".");
    }
    auto vec = duckdb_data_chunk_get_vector(_chunk, _i);
    return ColumnData<T, ColName>{
        .vec = vec,
        .data = static_cast<T*>(duckdb_vector_get_data(vec)),
        .validity = duckdb_vector_get_validity(vec)};
  }
};

template <class T>
struct MakeChunkPtrs {
  auto operator()(const Ref<DuckDBResult>& _res, duckdb_data_chunk _chunk) {
    return MakeChunkPtrs<chunk_ptrs_t<T>>{}(_res, _chunk);
  }
};

template <class T>
auto make_chunk_ptrs = MakeChunkPtrs<std::remove_cvref_t<T>>{};

}  // namespace sqlgen::duckdb

#endif
