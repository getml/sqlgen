#ifndef SQLGEN_DUCKDB_DUCKDBRESULT_HPP_
#define SQLGEN_DUCKDB_DUCKDBRESULT_HPP_

#include <duckdb.h>

#include <string>

#include "DuckDBConnection.hpp"

namespace sqlgen::duckdb {

class DuckDBResult {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  static Result<Ref<DuckDBResult>> make(const std::string& _query,
                                        const ConnPtr& _conn) {
    try {
      return Ref<DuckDBResult>::make(_query, _conn);
    } catch (const std::exception& e) {
      return error(e.what());
    }
  }

  DuckDBResult(const std::string& _query, const ConnPtr& _conn)
      : destroy_(false) {
    if (duckdb_query(_conn->conn(), _query.c_str(), &res_) == DuckDBError) {
      throw std::runtime_error(duckdb_result_error(&res_));
    }
    destroy_ = true;
  }

  ~DuckDBResult() {
    if (destroy_) {
      duckdb_destroy_result(&res_);
    }
  }

  DuckDBResult(const DuckDBResult& _other) = delete;

  DuckDBResult(DuckDBResult&& _other)
      : destroy_(_other.destroy_), res_(_other.res_) {
    _other.destroy_ = false;
  }

  DuckDBResult& operator=(const DuckDBResult& _other) = delete;

  DuckDBResult& operator=(DuckDBResult&& _other) {
    if (this == &_other) {
      return *this;
    }
    destroy_ = _other.destroy_;
    res_ = _other.res_;
    _other.destroy_ = false;
    return *this;
  }

  duckdb_result& res() { return res_; }

 private:
  bool destroy_;

  duckdb_result res_;
};

}  // namespace sqlgen::duckdb

#endif
