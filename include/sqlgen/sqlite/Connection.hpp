#ifndef SQLGEN_SQLITE_CONNECTION_HPP_
#define SQLGEN_SQLITE_CONNECTION_HPP_

#include <sqlite3.h>

#include <memory>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../Connection.hpp"
#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../dynamic/Write.hpp"
#include "to_sql.hpp"

namespace sqlgen::sqlite {

class Connection : public sqlgen::Connection {
  using ConnPtr = Ref<sqlite3>;
  using StmtPtr = std::shared_ptr<sqlite3_stmt>;

 public:
  Connection(const std::string& _fname)
      : stmt_(nullptr), conn_(make_conn(_fname)), transaction_started_(false) {}

  static rfl::Result<Ref<sqlgen::Connection>> make(
      const std::string& _fname) noexcept;

  Connection(const Connection& _other) = delete;

  Connection(Connection&& _other) noexcept;

  ~Connection();

  Result<Nothing> begin_transaction() noexcept final;

  Result<Nothing> commit() noexcept final;

  Result<Nothing> execute(const std::string& _sql) noexcept final;

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>&
          _data) noexcept;

  Connection& operator=(const Connection& _other) = delete;

  Connection& operator=(Connection&& _other) noexcept;

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query) final;

  Result<Nothing> rollback() noexcept final;

  std::string to_sql(const dynamic::Statement& _stmt) noexcept final {
    return sqlite::to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt) final;

  Result<Nothing> end_write() final;

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data) final;

 private:
  /// Generates the underlying connection.
  static ConnPtr make_conn(const std::string& _fname);

  /// Actually inserts data based on a prepared statement -
  /// used by both .insert(...) and .write(...).
  Result<Nothing> actual_insert(
      const std::vector<std::vector<std::optional<std::string>>>& _data,
      sqlite3_stmt* _stmt) const noexcept;

  /// Generates a prepared statment, usually for inserts.
  Result<StmtPtr> prepare_statement(const std::string& _sql) const noexcept;

 private:
  /// A prepared statement - needed for the read and write operations. Note that
  /// we have declared it before conn_, meaning it will be destroyed first.
  StmtPtr stmt_;

  /// The underlying sqlite3 connection.
  ConnPtr conn_;

  /// Whether a transaction has been started.
  bool transaction_started_;
};

}  // namespace sqlgen::sqlite

#endif
