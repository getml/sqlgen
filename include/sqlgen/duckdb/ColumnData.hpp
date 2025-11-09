#ifndef SQLGEN_DUCKDB_COLUMNDATA_HPP_
#define SQLGEN_DUCKDB_COLUMNDATA_HPP_

#include <duckdb.h>

namespace sqlgen::duckdb {

template <class T>
struct ColumnData {
  duckdb_vector vec;
  T *data;
  uint64_t *validity;

  bool is_not_null(idx_t _i) const {
    return (validity == nullptr) || duckdb_validity_row_is_valid(validity, _i);
  }
};

}  // namespace sqlgen::duckdb

#endif
