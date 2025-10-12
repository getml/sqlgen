#ifndef SQLGEN_DUCKDB_FROMCHUNKPTRS_HPP_
#define SQLGEN_DUCKDB_FROMCHUNKPTRS_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <type_traits>
#include <utility>

#include "../Result.hpp"
#include "../Tuple.hpp"
#include "ColumnData.hpp"
#include "chunk_ptrs_t.hpp"

namespace sqlgen::duckdb {

template <class T, class ChunkPtrsT>
struct FromChunkPtrs;

template <class T, class... Ts>
struct FromChunkPtrs<T, Tuple<ColumnData<Ts>...>> {
  Result<T> operator(const Tuple<ColumnData<Ts>...>& _chunk_ptrs, idx_t _i) {
    return [&]<int... _is>(std::integer_sequence<int, _is...>) {
      // TODO: Integrate this into the parser logic.
      return T{*(rfl::get<_is>(_chunk_ptrs).data + _i)...};
    }(std::make_integer_sequence<int, sizeof...(Ts)>());
  }
};

template <class T>
auto from_chunk_ptrs = FromChunkPtrs<std::remove_cvref_t<T>, chunk_ptrs_t<T>>{};

}  // namespace sqlgen::duckdb

#endif
