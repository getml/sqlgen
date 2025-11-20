#include "sqlgen/postgres/exec.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

namespace sqlgen::postgres {

Result<Ref<PGresult>> exec(const Ref<PGconn>& _conn,
                           const std::string& _sql) noexcept {
  const auto res = PQexec(_conn.get(), _sql.c_str());

  const auto status = PQresultStatus(res);

  if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK &&
      status != PGRES_COPY_IN) {
    const auto err =
        error("Executing '" + _sql + "' failed: " + PQresultErrorMessage(res));
    PQclear(res);
    return err;
  }

  return Ref<PGresult>::make(std::shared_ptr<PGresult>(res, PQclear));
}

}  // namespace sqlgen::postgres
