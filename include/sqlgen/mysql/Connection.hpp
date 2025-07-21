#ifndef SQLGEN_MYSQL_CONNECTION_HPP_
#define SQLGEN_MYSQL_CONNECTION_HPP_

#include <mysql.h>

#include <memory>
#include <rfl.hpp>
#include <stdexcept>
#include <string>

#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../Transaction.hpp"
#include "../dynamic/Column.hpp"
#include "../dynamic/Statement.hpp"
#include "../dynamic/Write.hpp"
#include "../is_connection.hpp"
#include "Credentials.hpp"
#include "exec.hpp"
#include "to_sql.hpp"

namespace sqlgen::mysql {

class Connection {
  using ConnPtr = Ref<MYSQL>;

 public:
  Connection(const Credentials& _credentials)
      : conn_(make_conn(_credentials)), credentials_(_credentials) {}

  static rfl::Result<Ref<Connection>> make(
      const Credentials& _credentials) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string& _sql) noexcept {
    // return exec(conn_, _sql).transform([](auto&&) { return Nothing{}; });

    return error("TODO");
  }

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>&
          _data) noexcept;

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query);

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    // return mysql::to_sql_impl(_stmt);
    return "TODO";
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt) {
    // return execute(mysql::to_sql_impl(_stmt));
    return error("TODO");
  }

  Result<Nothing> end_write();

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data);

 private:
  static ConnPtr make_conn(const Credentials& _credentials);

  static rfl::Unexpected<Error> make_error(const ConnPtr& _conn) noexcept;

 private:
  ConnPtr conn_;

  Credentials credentials_;
};

static_assert(is_connection<Connection>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Transaction<Connection>>,
              "Must fulfill the is_connection concept.");

}  // namespace sqlgen::mysql

#endif
