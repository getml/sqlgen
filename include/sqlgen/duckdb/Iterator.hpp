#ifndef SQLGEN_DUCKDB_ITERATOR_HPP_
#define SQLGEN_DUCKDB_ITERATOR_HPP_

#include <duckdb.h>

#include <optional>
#include <rfl.hpp>
#include <string>
#include <vector>

#include "../Ref.hpp"
#include "../Result.hpp"
#include "DuckDBConnection.hpp"
#include "from_chunk_ptrs.hpp"
#include "make_chunk_ptrs.hpp"

namespace sqlgen::duckdb {

template <class T>
class Iterator {
  using ConnPtr = Ref<DuckDBConnection>;
  using ResultPtr = Ref<duckdb_result>;

 public:
  struct End {
    bool operator==(const Iterator<T>& _it) const noexcept {
      return _it == *this;
    }

    bool operator!=(const Iterator<T>& _it) const noexcept {
      return _it != *this;
    }
  };

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Result<T>;

  Iterator(const ResultPtr& _res, const ConnPtr& _conn)
      : res_(_res),
        conn_(_conn),
        current_batch_(get_next_batch(_res, _conn)),
        ix_(0) {}

  ~Iterator() = default;

  Result<T>& operator*() const noexcept { return (*current_batch_)[ix_]; }

  Result<T>* operator->() const noexcept { return &(*current_batch_)[ix_]; }

  bool operator==(const End&) const noexcept {
    return current_batch_->size() == 0;
  }

  bool operator!=(const End& _end) const noexcept { return !(*this == _end); }

  Iterator<T>& operator++() noexcept {
    ++ix_;
    if (ix_ >= current_batch_->size()) {
      current_batch_ = get_next_batch(res_, conn_);
      ix_ = 0;
    }
    return *this;
  }

  void operator++(int) noexcept { ++*this; }

 private:
  static Ref<std::vector<Result<T>>> get_next_batch(
      const ResultPtr& _res, const ConnPtr& _conn) noexcept {
    duckdb_data_chunk chunk = duckdb_fetch_chunk(*_res);
    if (!chunk) {
      return Ref<std::vector<Result<T>>>::make();
    }
    const idx_t row_count = duckdb_data_chunk_get_size(chunk);
    return make_chunk_ptrs<T>(chunk)
        .transform([&](auto&& _chunk_ptrs) {
          auto batch = Ref<std::vector<Result<T>>>::make();
          for (idx_t i = 0; i < row_count; ++i) {
            batch->emplace_back(from_chunk_ptrs<T>(_chunk_ptrs, i));
          }
          return batch;
        })
        .or_else([](auto _err) {
          return Ref<std::vector<Result<T>>>::make(
              std::vector<Result<T>>({Result<T>(_err)}));
        })
        .value();
  }

 private:
  /// The underlying DuckDB result.
  ResultPtr res_;

  /// The underlying connection.
  ConnPtr conn_;

  /// The current batch of results.
  Ref<std::vector<Result<T>>> current_batch_;

  /// The index on the current_chunk
  idx_t ix_;
};

}  // namespace sqlgen::duckdb

#endif
