#ifndef SQLGEN_TRANSACTION_HPP_
#define SQLGEN_TRANSACTION_HPP_

#include "Ref.hpp"
#include "is_connection.hpp"

namespace sqlgen {

template <class _ConnType>
  requires is_connection<_ConnType>
class Transaction {
 public:
  using ConnType = _ConnType;

  Transaction(const Ref<ConnType>& _conn) : conn_(_conn) {}

  Transaction(const Transaction& _other) = delete;

  ~Transaction() { conn_->rollback(); }

  Result<Nothing> begin_transaction() { return conn_->begin_transaction(); }

  Result<Nothing> commit() { return conn_->commit(); }

  const Ref<ConnType>& conn() const noexcept { return conn_; }

  Result<Nothing> execute(const std::string& _sql) {
    return conn_->execute(_sql);
  }

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>& _data) {
    return conn_->insert(_stmt, _data);
  }

  Transaction operator=(const Transaction& _other) = delete;

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query) {
    return conn_->read(_query);
  }

  Result<Nothing> rollback() noexcept {
    return error(
        "You should never call .rollback() on a transaction object, just let "
        "it go out of scope instead.");
  }

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return conn_->to_sql(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt) {
    return conn_->to_sql(_stmt);
  }

  Result<Nothing> end_write() { return conn_->end_write(); }

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data) {
    return conn_->write(_data);
  }

 private:
  Ref<ConnType> conn_;
};

}  // namespace sqlgen

#endif
