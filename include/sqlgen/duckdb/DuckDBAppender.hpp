#ifndef SQLGEN_DUCKDB_DUCKDBAPPENDER_HPP_
#define SQLGEN_DUCKDB_DUCKDBAPPENDER_HPP_

#include <duckdb.h>

#include <string>

#include "DuckDBConnection.hpp"

namespace sqlgen::duckdb {

class DuckDBAppender {
  using ConnPtr = Ref<DuckDBConnection>;

 public:
  static Result<Ref<DuckDBAppender>> make(
      const std::string& _sql, const ConnPtr& _conn,
      const std::vector<const char*>& _columns,
      const std::vector<duckdb_logical_type>& _types) {
    try {
      return Ref<DuckDBAppender>::make(_sql, _conn, _columns, _types);
    } catch (const std::exception& e) {
      return error(e.what());
    }
  }

  DuckDBAppender(const std::string& _sql, const ConnPtr& _conn,
                 std::vector<const char*> _columns,
                 std::vector<duckdb_logical_type> _types)
      : destroy_(false) {
    if (duckdb_appender_create_query(
            _conn->conn(), _sql.c_str(), static_cast<idx_t>(_columns.size()),
            _types.data(), "sqlgen_appended_data", _columns.data(),
            &appender_) == DuckDBError) {
      throw std::runtime_error("Could not create appender.");
    }
    destroy_ = true;
  }

  ~DuckDBAppender() {
    if (destroy_) {
      duckdb_appender_destroy(&appender_);
    }
  }

  DuckDBAppender(const DuckDBAppender& _other) = delete;

  DuckDBAppender(DuckDBAppender&& _other)
      : destroy_(_other.destroy_), appender_(_other.appender_) {
    _other.destroy_ = false;
  }

  DuckDBAppender& operator=(const DuckDBAppender& _other) = delete;

  DuckDBAppender& operator=(DuckDBAppender&& _other) {
    if (this == &_other) {
      return *this;
    }
    destroy_ = _other.destroy_;
    appender_ = _other.appender_;
    _other.destroy_ = false;
    return *this;
  }

  duckdb_appender& appender() { return appender_; }

 private:
  bool destroy_;

  duckdb_appender appender_;
};

}  // namespace sqlgen::duckdb

#endif
