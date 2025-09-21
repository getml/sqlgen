#include "sqlgen/duckdb/Iterator.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>

#include "sqlgen/duckdb/exec.hpp"
#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"

namespace sqlgen::duckdb {

Iterator::Iterator(const ResultPtr& _res, const ConnPtr& _conn)
    : res_(_res),
      conn_(_conn),
      num_cols_(duckdb_row_count(*_res)),
      num_rows_(duckdb_column_count(*_res)),
      rownum_(0) {}

bool Iterator::end() const { return rownum_ >= num_rows_; }

Result<std::vector<std::vector<std::optional<std::string>>>> Iterator::next(
    const size_t _batch_size) {
  auto vec = std::vector<std::vector<std::optional<std::string>>>();

  if (end()) {
    return vec;
  }

  const auto batch_size =
      std::min(num_rows_ - rownum_, static_cast<idx_t>(_batch_size));

  for (idx_t i = 0; i < batch_size; ++i, ++rownum_) {
    auto row = std::vector<std::optional<std::string>>();
    for (idx_t col = 0; col < num_cols_; ++col) {
      auto str_val = duckdb_value_varchar(&result, col, rownum_);
      if (str_val) {
        duckdb_free(str_val);
      } else {
      }
    }
  }
}

}  // namespace sqlgen::duckdb
