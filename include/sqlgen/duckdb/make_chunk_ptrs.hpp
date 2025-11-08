#ifndef SQLGEN_DUCKDB_MAKECHUNKPTRS_HPP_
#define SQLGEN_DUCKDB_MAKECHUNKPTRS_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <type_traits>
#include <utility>

#include "ColumnData.hpp"
#include "chunk_ptrs_t.hpp"

namespace sqlgen::duckdb {

template <class T>
struct MakeChunkPtrs;

template <class... Ts>
struct MakeChunkPtrs<rfl::Tuple<ColumnData<Ts>...>> {
  Result<rfl::Tuple<ColumnData<Ts>...>> operator()(duckdb_data_chunk _chunk) {
    // TODO: Runtime type checking
    return [&]<int... _is>(std::integer_sequence<int, _is...>) {
      return rfl::Tuple<ColumnData<Ts>...>(
          make_column_data<Ts, _is>(_chunk)...);
    }(std::make_integer_sequence<int, sizeof...(Ts)>());
  }

  template <class T, int _i>
  static auto make_column_data(duckdb_data_chunk _chunk) {
    auto vec = duckdb_data_chunk_get_vector(_chunk, _i);
    return ColumnData<T>{.vec = vec,
                         .data = static_cast<T*>(duckdb_vector_get_data(vec)),
                         .validity = duckdb_vector_get_validity(vec)};
  }
};

template <class T>
struct MakeChunkPtrs {
  auto operator()(duckdb_data_chunk _chunk) {
    return MakeChunkPtrs<chunk_ptrs_t<T>>{}(_chunk);
  }
};

template <class T>
auto make_chunk_ptrs = MakeChunkPtrs<std::remove_cvref_t<T>>{};

}  // namespace sqlgen::duckdb

#endif
