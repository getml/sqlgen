#ifndef SQLGEN_CONNECTIONPOOL_HPP_
#define SQLGEN_CONNECTIONPOOL_HPP_

#include <atomic>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include "Ref.hpp"
#include "Result.hpp"
#include "Session.hpp"

namespace sqlgen {

template <class Connection>
class ConnectionPool {
  using ConnPtr = Ref<Connection>;

 public:
  template <class... Args>
  ConnectionPool(const size_t _size, const Args&... _args) {
    conns_->reserve(_size);
    for (size_t i = 0; i < _size; ++i) {
      auto conn = Ref<Connection>::make(_args...);
      auto flag = Ref<std::atomic_flag>::make();
      flag->clear();
      conns_->emplace_back(std::make_pair(std::move(conn), std::move(flag)));
    }
  }

  template <class... Args>
  static Result<ConnectionPool> make(const size_t _size,
                                     const Args&... _args) noexcept {
    try {
      return ConnectionPool(_size, _args...);
    } catch (std::exception& e) {
      return error(e.what());
    }
  }

  ~ConnectionPool() = default;

  /// Acquire a session from the pool. Returns an error if no connections are
  /// available.
  Result<Ref<Session<Connection>>> acquire() noexcept {
    for (auto& [conn, flag] : *conns_) {
      if (!flag->test_and_set()) {
        return Ref<Session<Connection>>::make(conn, flag);
      }
    }
    return error("No available connections in the pool.");
  }

  /// Get the current number of available connections
  size_t available() const {
    return std::accumulate(conns_->begin(), conns_->end(), 0,
                           [](const auto _count, const auto& _p) {
                             return _p.second->test() ? _count : _count + 1;
                           });
  }

  /// Get the total number of connections in the pool
  size_t size() const { return conns_->size(); }

 private:
  /// The underlying connection objects.
  Ref<std::vector<std::pair<ConnPtr, Ref<std::atomic_flag>>>> conns_;
};

template <class Connection, class... Args>
Result<ConnectionPool<Connection>> make_connection_pool(
    const size_t _size, const Args&... _args) noexcept {
  return ConnectionPool<Connection>::make(_size, _args...);
}

template <class Connection>
Result<Ref<Session<Connection>>> session(
    ConnectionPool<Connection> _pool) noexcept {
  return _pool.acquire();
}

template <class Connection>
Result<Ref<Session<Connection>>> session(
    Result<ConnectionPool<Connection>> _res) noexcept {
  return _res.and_then([](auto _pool) { return session(_pool); });
}

}  // namespace sqlgen

#endif
