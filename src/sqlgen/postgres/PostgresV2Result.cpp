#include "sqlgen/postgres/PostgresV2Connection.hpp"

namespace sqlgen::postgres {

rfl::Result<PostgresV2Result> PostgresV2Result::make(
    const std::string& _query, const PostgresV2Connection& _conn) noexcept {
  auto res = PQexec(_conn.ptr(), _query.c_str());
  const auto status = PQresultStatus(res);
  if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK &&
      status != PGRES_COPY_IN) {
    const auto msg =
        std::string("Query execution failed: ") + PQerrorMessage(_conn.ptr());
    PQclear(res);
    return error(msg);
  }
  return PostgresV2Result(res);
}

}  // namespace sqlgen::postgres
