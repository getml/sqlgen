#include "sqlgen/postgres/Connection.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/postgres/Iterator.hpp"

namespace sqlgen::postgres {

Result<Nothing> Connection::begin_transaction() noexcept {
  return execute("BEGIN TRANSACTION;");
}

Result<Nothing> Connection::commit() noexcept { return execute("COMMIT;"); }

Result<Nothing> Connection::end_write() {
  if (PQputCopyEnd(conn_.get(), NULL) == -1) {
    return error(PQerrorMessage(conn_.get()));
  }
  const auto res = PQgetResult(conn_.get());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    return error(PQerrorMessage(conn_.get()));
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

  const auto sql = to_sql_impl(_stmt);

  const auto res = execute("PREPARE \"sqlgen_insert_into_table\" AS " + sql);

  if (!res) {
    return res;
  }

  std::vector<const char*> current_row(_data[0].size());

  const int n_params = static_cast<int>(current_row.size());

  for (size_t i = 0; i < _data.size(); ++i) {
    const auto& d = _data[i];

    if (d.size() != current_row.size()) {
      execute("DEALLOCATE sqlgen_insert_into_table;");
      return error("Error in entry " + std::to_string(i) + ": Expected " +
                   std::to_string(current_row.size()) + " entries, got " +
                   std::to_string(d.size()));
    }

    for (size_t j = 0; j < d.size(); ++j) {
      current_row[j] = d[j] ? d[j]->c_str() : nullptr;
    }

    const auto res = PQexecPrepared(conn_.get(),                 // conn
                                    "sqlgen_insert_into_table",  // stmtName
                                    n_params,                    // nParams
                                    current_row.data(),          // paramValues
                                    nullptr,                     // paramLengths
                                    nullptr,                     // paramFormats
                                    0                            // resultFormat
    );

    const auto status = PQresultStatus(res);

    if (status != PGRES_COMMAND_OK) {
      const auto err = error(std::string("Executing INSERT failed: ") +
                             PQresultErrorMessage(res));
      execute("DEALLOCATE sqlgen_insert_into_table;");
      return err;
    }
  }

  return execute("DEALLOCATE sqlgen_insert_into_table;");
}

rfl::Result<Ref<Connection>> Connection::make(
    const Credentials& _credentials) noexcept {
  try {
    return Ref<Connection>::make(_credentials);
  } catch (std::exception& e) {
    return error(e.what());
  }
}

typename Connection::ConnPtr Connection::make_conn(
    const std::string& _conn_str) {
  const auto raw_ptr = PQconnectdb(_conn_str.c_str());

  if (PQstatus(raw_ptr) != CONNECTION_OK) {
    const auto msg = std::string("Connection to postgres failed: ") +
                     PQerrorMessage(raw_ptr);
    PQfinish(raw_ptr);
    throw std::runtime_error(msg.c_str());
  }

  return ConnPtr::make(std::shared_ptr<PGconn>(raw_ptr, &PQfinish)).value();
}

Result<Ref<IteratorBase>> Connection::read_impl(
    const dynamic::SelectFrom& _query) {
  const auto sql = postgres::to_sql_impl(_query);
  try {
    return Ref<IteratorBase>(Ref<Iterator>::make(sql, conn_));
  } catch (std::exception& e) {
    return error(e.what());
  }
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

Result<Nothing> Connection::write_impl(
    const std::vector<std::vector<std::optional<std::string>>>& _data) {
  for (const auto& line : _data) {
    const auto buffer = to_buffer(line);
    const auto success = PQputCopyData(conn_.get(), buffer.c_str(),
                                       static_cast<int>(buffer.size()));
    if (success != 1) {
      PQputCopyEnd(conn_.get(), NULL);
      return error("Error occurred while writing data to postgres.");
    }
  }
  return Nothing{};
}

}  // namespace sqlgen::postgres
