#include "sqlgen/postgres/PostgresV2Connection.hpp"

namespace sqlgen::postgres {

rfl::Result<PostgresV2Connection> PostgresV2Connection::make(
    const std::string& _conn_str) noexcept {
  auto conn = PQconnectdb(_conn_str.c_str());
  if (PQstatus(conn) != CONNECTION_OK) {
    const auto msg =
        std::string("Connection to postgres failed: ") + PQerrorMessage(conn);
    PQfinish(conn);
    return error(msg);
  }
  return PostgresV2Connection(conn);
}

}  // namespace sqlgen::postgres
