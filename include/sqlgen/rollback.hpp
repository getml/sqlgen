#ifndef SQLGEN_ROLLBACK_HPP_
#define SQLGEN_ROLLBACK_HPP_

#include <type_traits>

#include "Ref.hpp"
#include "Result.hpp"
#include "is_connection.hpp"

namespace sqlgen {

template <class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> rollback_impl(const Ref<Connection>& _conn) {
  return _conn->rollback().transform([&](const auto&) { return _conn; });
}

template <class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> rollback_impl(const Result<Ref<Connection>>& _res) {
  return _res.and_then([&](const auto& _conn) { return rollback_impl(_conn); });
}

struct Rollback {
  auto operator()(const auto& _conn) const noexcept {
    try {
      return rollback_impl(_conn);
    } catch (std::exception& e) {
      return error(e.what());
    }
  }
};

inline const auto rollback = Rollback{};

}  // namespace sqlgen

#endif
