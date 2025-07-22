#include "sqlgen/mysql/Connection.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/mysql/Iterator.hpp"
#include "sqlgen/mysql/make_error.hpp"

namespace sqlgen::mysql {

Result<Nothing> Connection::deallocate_prepared_insert_statement() noexcept {
  return execute("DEALLOCATE PREPARE `sqlgen_insert_into_table`;");
}

Result<Nothing> Connection::insert(
    const dynamic::Insert& _stmt,
    const std::vector<std::vector<std::optional<std::string>>>&
        _data) noexcept {
  if (_data.size() == 0) {
    return Nothing{};
  }
  return prepare_insert_statement(_stmt)
      .and_then([&](auto&&) { return write(_data); })
      .and_then([&](auto&&) { return deallocate_prepared_insert_statement(); });
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

Result<Nothing> Connection::prepare_insert_statement(
    const std::variant<dynamic::Insert, dynamic::Write>& _stmt) noexcept {
  const auto sql = std::visit(to_sql_impl, _stmt);
  return execute("PREPARE `sqlgen_insert_into_table` FROM " + sql);
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

Result<Nothing> Connection::start_write(const dynamic::Write& _stmt) {
  return begin_transaction().and_then(
      [&](auto&&) { return prepare_insert_statement(_stmt); });
}

Result<Nothing> Connection::write(
    const std::vector<std::vector<std::optional<std::string>>>& _data) {
  return [&]() -> Result<Nothing> { return Nothing{}; }().or_else(
                   [&](const auto& _err) {
                     deallocate_prepared_insert_statement();
                     return error(_err.what());
                   });
}

Result<Nothing> Connection::end_write() {
  return deallocate_prepared_insert_statement().and_then(
      [&](auto&&) { return commit(); });
}

}  // namespace sqlgen::mysql
