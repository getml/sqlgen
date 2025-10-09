#ifndef SQLGEN_MYSQL_CONNECTION_HPP_
#define SQLGEN_MYSQL_CONNECTION_HPP_

#include <mysql.h>

#include <memory>
#include <rfl.hpp>
#include <stdexcept>
#include <string>

#include "../Iterator.hpp"
#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../Transaction.hpp"
#include "../dynamic/Column.hpp"
#include "../dynamic/Statement.hpp"
#include "../dynamic/Write.hpp"
#include "../internal/to_container.hpp"
#include "../internal/write_or_insert.hpp"
#include "../is_connection.hpp"
#include "../transpilation/value_t.hpp"
#include "Credentials.hpp"
#include "exec.hpp"
#include "to_sql.hpp"

namespace sqlgen::mysql {

class Connection {
  using ConnPtr = Ref<MYSQL>;
  using StmtPtr = std::shared_ptr<MYSQL_STMT>;

 public:
  Connection(const Credentials& _credentials)
      : conn_(make_conn(_credentials)) {}

  static rfl::Result<Ref<Connection>> make(
      const Credentials& _credentials) noexcept;

  ~Connection() = default;

  Result<Nothing> begin_transaction() noexcept {
    return execute("START TRANSACTION;");
  }

  Result<Nothing> commit() noexcept { return execute("COMMIT;"); }

  Result<Nothing> execute(const std::string& _sql) noexcept {
    return exec(conn_, _sql);
  }

  template <class ItBegin, class ItEnd>
  Result<Nothing> insert(const dynamic::Insert& _stmt, ItBegin _begin,
                         ItEnd _end) noexcept {
    return internal::write_or_insert(
        [&](const auto& _data) { return insert_impl(_stmt, _data); }, _begin,
        _end);
  }

  template <class ContainerType>
  auto read(const dynamic::SelectFrom& _query) {
    using ValueType = transpilation::value_t<ContainerType>;
    return internal::to_container<ContainerType>(read_impl(_query).transform(
        [](auto&& _it) { return Iterator<ValueType>(std::move(_it)); }));
  }

  Result<Nothing> rollback() noexcept { return execute("ROLLBACK;"); }

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return to_sql_impl(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt);

  template <class ItBegin, class ItEnd>
  Result<Nothing> write(ItBegin _begin, ItEnd _end) {
    return internal::write_or_insert(
        [&](const auto& _data) { return write_impl(_data); }, _begin, _end);
  }

  Result<Nothing> end_write();

 private:
  /// Actually inserts data based on a prepared statement -
  /// used by both .insert(...) and .write(...).
  Result<Nothing> actual_insert(
      const std::vector<std::vector<std::optional<std::string>>>& _data,
      MYSQL_STMT* _stmt) const noexcept;

  Result<Nothing> insert_impl(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>&
          _data) noexcept;

  static ConnPtr make_conn(const Credentials& _credentials);

  Result<StmtPtr> prepare_statement(
      const std::variant<dynamic::Insert, dynamic::Write>& _stmt)
      const noexcept;

  Result<Ref<IteratorBase>> read_impl(const dynamic::SelectFrom& _query);

  Result<Nothing> write_impl(
      const std::vector<std::vector<std::optional<std::string>>>& _data);

 private:
  /// A prepared statement - needed for the read and write operations. Note that
  /// we have declared it before conn_, meaning it will be destroyed first.
  StmtPtr stmt_;

  /// The underlying connection.
  ConnPtr conn_;
};

static_assert(is_connection<Connection>,
              "Must fulfill the is_connection concept.");
static_assert(is_connection<Transaction<Connection>>,
              "Must fulfill the is_connection concept.");

}  // namespace sqlgen::mysql

#endif
