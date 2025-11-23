#include "sqlgen/postgres/Connection.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/random.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/postgres/Iterator.hpp"
#include "sqlgen/postgres/PostgresV2Result.hpp"

namespace sqlgen::postgres {

Connection::Connection(const Conn& _conn) : conn_(_conn) {}

Connection::Connection(const Credentials& _credentials)
    : conn_(PostgresV2Connection::make(_credentials.to_str()).value()) {}

Result<Nothing> Connection::begin_transaction() noexcept {
  return execute("BEGIN TRANSACTION;");
}

Result<Nothing> Connection::commit() noexcept { return execute("COMMIT;"); }

Result<Nothing> Connection::execute(const std::string& _sql) noexcept {
  return PostgresV2Result::make(_sql, conn_).transform([](auto&&) {
    return Nothing{};
  });
}

Result<Nothing> Connection::end_write() {
  if (PQputCopyEnd(conn_.ptr(), NULL) == -1) {
    return error(PQerrorMessage(conn_.ptr()));
  }
  const auto res = PostgresV2Result(PQgetResult(conn_.ptr()));
  if (PQresultStatus(res.ptr()) != PGRES_COMMAND_OK) {
    return error(PQerrorMessage(conn_.ptr()));
  }
  return Nothing{};
}

Result<Nothing> Connection::insert_impl(
    const dynamic::Insert& _stmt,
    const std::vector<std::vector<std::optional<std::string>>>&
        _data) noexcept {
  if (_data.size() == 0) {
    return Nothing{};
  }

  const auto name = "sqlgen_insert_into_table_" + internal::random();

  const auto sql = to_sql_impl(_stmt);

  const auto res = PostgresV2Result(PQprepare(
      conn_.ptr(), name.c_str(), sql.c_str(), _data.at(0).size(), nullptr));

  const auto status = PQresultStatus(res.ptr());

  if (status != PGRES_COMMAND_OK) {
    return error("Generating prepared statement for '" + sql +
                 "' failed: " + PQresultErrorMessage(res.ptr()));
  }

  std::vector<const char*> current_row(_data[0].size());

  const int n_params = static_cast<int>(current_row.size());

  for (size_t i = 0; i < _data.size(); ++i) {
    const auto& d = _data[i];

    if (d.size() != current_row.size()) {
      execute("DEALLOCATE " + name + ";");
      return error("Error in entry " + std::to_string(i) + ": Expected " +
                   std::to_string(current_row.size()) + " entries, got " +
                   std::to_string(d.size()));
    }

    for (size_t j = 0; j < d.size(); ++j) {
      current_row[j] = d[j] ? d[j]->c_str() : nullptr;
    }

    const auto res =
        PostgresV2Result(PQexecPrepared(conn_.ptr(),         // conn
                                        name.c_str(),        // stmtName
                                        n_params,            // nParams
                                        current_row.data(),  // paramValues
                                        nullptr,             // paramLengths
                                        nullptr,             // paramFormats
                                        0                    // resultFormat
                                        ));

    const auto status = PQresultStatus(res.ptr());

    if (status != PGRES_COMMAND_OK) {
      const auto err = error(std::string("Executing INSERT failed: ") +
                             PQresultErrorMessage(res.ptr()));
      execute("DEALLOCATE " + name + ";");
      return err;
    }
  }

  return execute("DEALLOCATE " + name + ";");
}

rfl::Result<Ref<Connection>> Connection::make(
    const Credentials& _credentials) noexcept {
  return PostgresV2Connection::make(_credentials.to_str())
      .transform([](auto&& _conn) { return Ref<Connection>::make(_conn); });
}

Result<Ref<Iterator>> Connection::read_impl(
    const rfl::Variant<dynamic::SelectFrom, dynamic::Union>& _query) {
  const auto sql = _query.visit([](const auto& _q) { return to_sql_impl(_q); });
  return Ref<Iterator>::make(sql, conn_);
}

Result<Nothing> Connection::rollback() noexcept { return execute("ROLLBACK;"); }

std::string Connection::to_buffer(
    const std::vector<std::optional<std::string>>& _line) const noexcept {
  using namespace std::ranges::views;

  const auto edit_field =
      [](const std::optional<std::string>& _field) -> std::string {
    if (!_field) {
      return "\e";
    }
    if (_field->find("\t") != std::string::npos) {
      return "\a" + *_field + "\a";
    }
    return *_field;
  };

  return internal::strings::join(
             "\t", internal::collect::vector(_line | transform(edit_field))) +
         "\n";
}

std::string Connection::to_sql(const dynamic::Statement& _stmt) noexcept {
  return postgres::to_sql_impl(_stmt);
}

Result<Nothing> Connection::start_write(const dynamic::Write& _stmt) {
  return execute(postgres::to_sql_impl(_stmt));
}

Result<Nothing> Connection::write_impl(
    const std::vector<std::vector<std::optional<std::string>>>& _data) {
  for (const auto& line : _data) {
    const auto buffer = to_buffer(line);
    const auto success = PQputCopyData(conn_.ptr(), buffer.c_str(),
                                       static_cast<int>(buffer.size()));
    if (success != 1) {
      PQputCopyEnd(conn_.ptr(), NULL);
      return error("Error occurred while writing data to postgres.");
    }
  }
  return Nothing{};
}

}  // namespace sqlgen::postgres

