#ifndef SQLGEN_DUCKDB_COLUMNDATA_HPP_
#define SQLGEN_DUCKDB_COLUMNDATA_HPP_

#include <duckdb.h>

namespace sqlgen::duckdb {

template <class T>
struct ColumnData {
  duckdb_vector vec;
  T *data;
  uint64_t *validity;
};

}  // namespace sqlgen::duckdb

#endif
