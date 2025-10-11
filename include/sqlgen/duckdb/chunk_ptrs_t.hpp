#ifndef SQLGEN_DUCKDB_CHUNKPTRST_HPP_
#define SQLGEN_DUCKDB_CHUNKPTRST_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <type_traits>

namespace sqlgen::duckdb {

template <class T>
struct ColumnData {
  duckdb_vector vec;
  T *data;
  uint64_t *validity;
};

template <class T>
struct ChunkPtrsType;

template <class... FieldTs>
struct ChunkPtrsType<rfl::NamedTuple<FieldTs...>> {
  using Type = rfl::Tuple<ColumnData<typename FieldTs::Type>...>;
};

template <class T>
struct ChunkPtrsType<T> {
  using Type = typename ChunkPtrsType<rfl::named_tuple_t<T>>::Type;
};

template <class T>
using chunk_ptrs_t = typename ChunkPtrsType<std::remove_cvref_t<T>>::Type;

}  // namespace sqlgen::duckdb

#endif
