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

  Result<Nothing> begin_transaction() noexcept {
    return execute("BEGIN TRANSACTION;");
  }

  Result<Nothing> commit() noexcept { return execute("COMMIT;"); }

  Result<Nothing> execute(const std::string& _sql) noexcept {
    return exec(conn_, _sql).transform([](auto&&) { return Nothing{}; });
  }

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>&
          _data) noexcept;

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query);

  Result<Nothing> rollback() noexcept { return execute("ROLLBACK;"); }

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt);

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data);

  Result<Nothing> end_write();

 private:
  Result<Nothing> deallocate_prepared_insert_statement() noexcept;

  static ConnPtr make_conn(const Credentials& _credentials);

  Result<Nothing> prepare_insert_statement(
      const std::variant<dynamic::Insert, dynamic::Write>& _stmt) noexcept;

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
