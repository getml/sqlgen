#ifndef SQLGEN_DUCKDB_ITERATOR_HPP_
#define SQLGEN_DUCKDB_ITERATOR_HPP_

#include <duckdb.h>

#include <optional>
#include <string>
#include <vector>

#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "DuckDBConnection.hpp"

namespace sqlgen::sqlite {

class Iterator : public sqlgen::IteratorBase {
  using ConnPtr = Ref<DuckDBConnection>;
  using ResultPtr = Ref<duckdb_result>;

 public:
  Iterator(const ResultPtr& _res, const ConnPtr& _conn);

  ~Iterator();

  /// Whether the end of the available data has been reached.
  bool end() const final;

  /// Returns the next batch of rows.
  /// If _batch_size is greater than the number of rows left, returns all
  /// of the rows left.
  Result<std::vector<std::vector<std::optional<std::string>>>> next(
      const size_t _batch_size) final;

 private:
  static void destroy_result(duckdb_result* _r) {
    duckdb_result(_r);
    delete _r;
  }

 private:
  /// The underlying DuckDB result.
  ResultPtr res_;

  /// The underlying connection.
  ConnPtr conn_;

  /// The number of columns.
  idx_t num_cols_;

  /// The number of rows.
  idx_t num_rows_;

  /// The current rownumber.
  idx_t rownum_;
};

}  // namespace sqlgen::sqlite

#endif
