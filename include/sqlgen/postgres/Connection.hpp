#ifndef SQLGEN_POSTGRES_CONNECTION_HPP_
#define SQLGEN_POSTGRES_CONNECTION_HPP_

#include <libpq-fe.h>

#include <memory>
#include <rfl.hpp>
#include <stdexcept>
#include <string>

#include "../Connection.hpp"
#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../dynamic/Column.hpp"
#include "../dynamic/Statement.hpp"
#include "Credentials.hpp"
#include "exec.hpp"
#include "to_sql.hpp"

namespace sqlgen::postgres {

class Connection : public sqlgen::Connection {
  using ConnPtr = Ref<PGconn>;

 public:
  Connection(const Credentials& _credentials)
      : conn_(make_conn(_credentials.to_str())),
        credentials_(_credentials),
        transaction_started_(false) {}

  static rfl::Result<Ref<sqlgen::Connection>> make(
      const Credentials& _credentials) noexcept;

  Connection(const Connection& _other) = delete;

  Connection(Connection&& _other) noexcept;

  ~Connection();

  Result<Nothing> begin_transaction() noexcept final;

  Result<Nothing> commit() noexcept final;

  Result<Nothing> execute(const std::string& _sql) noexcept final {
    return exec(conn_, _sql).transform([](auto&&) { return Nothing{}; });
  }

  Connection& operator=(const Connection& _other) = delete;

  Connection& operator=(Connection&& _other) noexcept;

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query) final;

  Result<Nothing> rollback() noexcept final;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept final {
    return postgres::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Insert& _stmt) final {
    return execute(postgres::to_sql_impl(_stmt));
  }

  Result<Nothing> end_write() final;

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data) final;

 private:
  static ConnPtr make_conn(const std::string& _conn_str);

  std::string to_buffer(
      const std::vector<std::optional<std::string>>& _line) const noexcept;

 private:
  ConnPtr conn_;

  Credentials credentials_;

  bool transaction_started_;
};

}  // namespace sqlgen::postgres

#endif
