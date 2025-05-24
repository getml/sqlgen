#ifndef SQLGEN_SESSION_HPP_
#define SQLGEN_SESSION_HPP_

#include <atomic>
#include <memory>
#include <optional>
#include <vector>

#include "IteratorBase.hpp"
#include "Ref.hpp"
#include "dynamic/Insert.hpp"
#include "dynamic/SelectFrom.hpp"
#include "dynamic/Statement.hpp"
#include "dynamic/Write.hpp"

namespace sqlgen {

template <class Connection>
class Session {
 public:
  using ConnPtr = Ref<Connection>;

  Session(const Ref<Connection>& _conn, const Ref<std::atomic_flag>& _flag)
      : conn_(_conn), flag_(_flag.ptr()) {}

  Session(const Session<Connection>& _other) = delete;

  Session(Session<Connection>&& _other)
      : conn_(std::move(_other.conn_)), flag_(_other.flag_) {
    _other.flag_.reset();
  }

  ~Session() {
    if (flag_) {
      flag_->clear();
    }
  }

  Result<Nothing> begin_transaction() { return conn_->begin_transaction(); }

  Result<Nothing> commit() { return conn_->commit(); }

  Result<Nothing> execute(const std::string& _sql) {
    return conn_->execute(_sql);
  }

  Result<Nothing> insert(
      const dynamic::Insert& _stmt,
      const std::vector<std::vector<std::optional<std::string>>>& _data) {
    return conn_->insert(_stmt, _data);
  }

  Session& operator=(const Session& _other) = delete;

  Session& operator=(Session&& _other) noexcept {
    if (this == &_other) {
      return *this;
    }
    conn_ = std::move(_other.conn_);
    flag_ = _other.flag_;
    _other.flag_.reset();
    return *this;
  }

  Result<Ref<IteratorBase>> read(const dynamic::SelectFrom& _query) {
    return conn_->read(_query);
  }

  Result<Nothing> rollback() noexcept { return conn_->rollback(); }

  std::string to_sql(const dynamic::Statement& _stmt) noexcept {
    return conn_->to_sql(_stmt);
  }

  Result<Nothing> start_write(const dynamic::Write& _stmt) {
    return conn_->start_write(_stmt);
  }

  Result<Nothing> end_write() { return conn_->end_write(); }

  Result<Nothing> write(
      const std::vector<std::vector<std::optional<std::string>>>& _data) {
    return conn_->write(_data);
  }

 private:
  /// The underlying connection object.
  ConnPtr conn_;

  /// The flag corresponding to the object - as long as this is true, we have
  /// ownership.
  std::shared_ptr<std::atomic_flag> flag_;
};

}  // namespace sqlgen

#endif
