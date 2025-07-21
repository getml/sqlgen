#include "sqlgen/mysql/Connection.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/mysql/Iterator.hpp"

namespace sqlgen::mysql {

Result<Nothing> Connection::begin_transaction() noexcept {
  // return execute("BEGIN TRANSACTION;");
  return Nothing{};
}

Result<Nothing> Connection::commit() noexcept {  // return execute("COMMIT;");
  return Nothing{};
}

Result<Nothing> Connection::end_write() {
  /*  if (PQputCopyEnd(conn_.get(), NULL) == -1) {
      return error(PQerrorMessage(conn_.get()));
    }
    const auto res = PQgetResult(conn_.get());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      return error(PQerrorMessage(conn_.get()));
    }
    return Nothing{};*/
  return Nothing{};
}

Result<Nothing> Connection::insert(
    const dynamic::Insert& _stmt,
    const std::vector<std::vector<std::optional<std::string>>>&
        _data) noexcept {
  /*  if (_data.size() == 0) {
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
                                      current_row.data(),          //
    paramValues nullptr,                     // paramLengths nullptr, //
    paramFormats 0                            // resultFormat
      );

      const auto status = PQresultStatus(res);

      if (status != PGRES_COMMAND_OK) {
        const auto err = error(std::string("Executing INSERT failed: ") +
                               PQresultErrorMessage(res));
        execute("DEALLOCATE sqlgen_insert_into_table;");
        return err;
      }
    }

    return execute("DEALLOCATE sqlgen_insert_into_table;");*/
  return Nothing{};
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
    const Credentials& _credentials) {
  const auto raw_ptr = mysql_init(nullptr);

  const auto shared_ptr = std::shared_ptr<MYSQL>(raw_ptr, mysql_close);

  const auto res = mysql_real_connect(
      shared_ptr.get(), _credentials.host.c_str(), _credentials.user.c_str(),
      _credentials.password.c_str(), _credentials.dbname.c_str(),
      _credentials.port, _credentials.unix_socket.c_str(),
      CLIENT_MULTI_STATEMENTS);

  if (!res) {
    throw std::runtime_error(
        make_error(ConnPtr::make(shared_ptr).value()).error().what());
  }

  return ConnPtr::make(shared_ptr).value();
}

rfl::Unexpected<Error> Connection::make_error(const ConnPtr& _conn) noexcept {
  const std::string msg =
      "MySQL error (" + std::to_string(mysql_errno(_conn.get())) + ") [" +
      mysql_sqlstate(_conn.get()) + "] " + mysql_error(_conn.get());
  return error(msg);
}

Result<Ref<IteratorBase>> Connection::read(const dynamic::SelectFrom& _query) {
  /*  const auto sql = mysql::to_sql_impl(_query);
    try {
      return Ref<IteratorBase>(Ref<Iterator>::make(sql, conn_));
    } catch (std::exception& e) {
      return error(e.what());
    }*/
  return error("TODO");
}

Result<Nothing>
Connection::rollback() noexcept { /*return execute("ROLLBACK;");*/
  return Nothing{};
}

Result<Nothing> Connection::write(
    const std::vector<std::vector<std::optional<std::string>>>& _data) {
  /*  for (const auto& line : _data) {
    const auto buffer = to_buffer(line);
    const auto success = PQputCopyData(conn_.get(), buffer.c_str(),
                                       static_cast<int>(buffer.size()));
    if (success != 1) {
      PQputCopyEnd(conn_.get(), NULL);
      return error("Error occurred while writing data to mysql.");
    }
  }*/
  return Nothing{};
}

}  // namespace sqlgen::mysql
