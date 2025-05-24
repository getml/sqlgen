#ifndef SQLGEN_POOL_SESSION_HPP_
#define SQLGEN_POOL_SESSION_HPP_

#include <atomic>
#include <memory>

#include "../Connection.hpp"
#include "../Ref.hpp"

namespace sqlgen::pool {

class Session : public Connection {
  using ConnPtr = Ref<Connection>;

  Session(const Ref<ConnType>& _conn,
          const std::shared_ptr<std::atomic_flag>& _flag)
      : conn_(_conn), flag_(_flag), transaction_started_(false) {}

  Session(const Session<ConnType>& _other) = delete;

  Session(Session<ConnType>&& _other) noexcept {}

  ~Session() {
    if (transaction_started_) {
      conn_->rollback();
    }
    if (flag_) {
      flag_->clear();
    }
  }

 private:
  /// The underlying connection object.
  ConnPtr conn_;

  /// The flag corresponding to the object - as long as this is true, we have
  /// ownership.
  std::shared_ptr<std::atomic_flag> flag_;

  /// Whether the user started a transaction.
  bool transaction_started_;
};

}  // namespace sqlgen::pool

#endif
