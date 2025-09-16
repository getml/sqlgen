#ifndef SQLGEN_DUCKDB_CONNECTION_HPP_
#define SQLGEN_DUCKDB_CONNECTION_HPP_

#include <duckdb.h>

#include <optional>
#include <string>

#include "../Result.hpp"

namespace sqlgen::duckdb {

class DuckDBConnection {
 public:
  Result<DuckDBConnection> make(const std::optional<std::string>& _fname);

  DuckDBConnection(duckdb_connection _conn, duckdb_database _db)
      : conn_(_conn), db_(_db) {}

  ~DuckDBConnection() {
    duckdb_disconnect(&conn_);
    duckdb_close(&db_);
  }

  DuckDBConnection(const DuckDBConnection& _other) = delete;

  DuckDBConnection(DuckDBConnection&& _other)
      : conn_(_other.conn_), db_(_other.db_) {
    _other.conn_ = NULL;
    _other.db_ = NULL;
  }

  DuckDBConnection& operator=(const DuckDBConnection& _other) = delete;

  DuckDBConnection& operator=(DuckDBConnection&& _other);

 private:
  duckdb_connection conn_;

  duckdb_database db_;
};

}  // namespace sqlgen::duckdb

#endif
