#include "sqlgen/duckdb/Connection.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

// #include "sqlgen/duckdb/Iterator.hpp"
#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"

namespace sqlgen::duckdb {

Result<Nothing> Connection::begin_transaction() noexcept {
  return execute("BEGIN TRANSACTION;");
}

Result<Nothing> Connection::commit() noexcept { return execute("COMMIT;"); }

Result<Nothing> Connection::end_write() {
  // TODO
  return error("TODO");
}

Result<Nothing> Connection::execute(const std::string& _sql) noexcept {
  duckdb_result res;
  const auto state = duckdb_query(conn_->conn(), _sql.c_str(), &res);
  if (state == DuckDBError) {
    const auto err = error(duckdb_result_error(&res));
    duckdb_destroy_result(&res);
    return err;
  }
  duckdb_destroy_result(&res);
  return Nothing{};
}

Result<Nothing> Connection::insert(
    const dynamic::Insert& _stmt,
    const std::vector<std::vector<std::optional<std::string>>>&
        _data) noexcept {
  // TODO
  return error("TODO");
}

rfl::Result<Ref<Connection>> Connection::make(
    const std::optional<std::string>& _fname) noexcept {
  return DuckDBConnection::make(_fname).transform(
      [](auto&& _conn) { return Ref<Connection>::make(std::move(_conn)); });
}

Result<Ref<IteratorBase>> Connection::read(const dynamic::SelectFrom& _query) {
  // TODO
  return error("TODO");
}

Result<Nothing> Connection::rollback() noexcept { return execute("ROLLBACK;"); }

Result<Nothing> Connection::write(
    const std::vector<std::vector<std::optional<std::string>>>& _data) {
  // TODO
  return error("TODO");
}

}  // namespace sqlgen::duckdb
