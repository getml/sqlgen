#ifndef SQLGEN_POSTGRES_CONNECTION_HPP_
#define SQLGEN_POSTGRES_CONNECTION_HPP_

#include <libpq-fe.h>

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
#include "../internal/to_container.hpp"
#include "../is_connection.hpp"
#include "Credentials.hpp"
#include "exec.hpp"
#include "to_sql.hpp"

namespace sqlgen::postgres {

class Connection {
  using ConnPtr = Ref<PGconn>;

 public:
  Connection(const Credentials& _credentials)
      : conn_(make_conn(_credentials.to_str())), credentials_(_credentials) {}

  static rfl::Result<Ref<Connection>> make(
      const Credentials& _credentials) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept;

  Result<Nothing> commit() noexcept;

  Result<Nothing> execute(const std::string& _sql) noexcept {
    return exec(conn_, _sql).transform([](auto&&) { return Nothing{}; });
  }

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>&
          _data) noexcept;

  template <class ContainerType>
  Result<ContainerType> read(const dynamic::SelectFrom& _query) {
    return internal::to_container<ContainerType>(read_impl(_query));
  }

  Result<Nothing> rollback() noexcept;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return postgres::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt) {
    return execute(postgres::to_sql_impl(_stmt));
  }

  Result<Nothing> end_write();

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data);

 private:
  static ConnPtr make_conn(const std::string& _conn_str);

  Result<Ref<IteratorBase>> read_impl(const dynamic::SelectFrom& _query);

  std::string to_buffer(
      const std::vector<std::optional<std::string>>& _line) const noexcept;

 private:
  ConnPtr conn_;

  Credentials credentials_;
};

static_assert(is_connection<Connection>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Transaction<Connection>>,
              "Must fulfill the is_connection concept.");

}  // namespace sqlgen::postgres

#endif
