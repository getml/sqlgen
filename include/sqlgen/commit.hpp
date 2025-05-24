#ifndef SQLGEN_COMMIT_HPP_
#define SQLGEN_COMMIT_HPP_

#include <type_traits>

#include "Ref.hpp"
#include "Result.hpp"
#include "is_connection.hpp"

namespace sqlgen {

template <class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> commit_impl(const Ref<Connection>& _conn) {
  return _conn->commit().transform([&](const auto&) { return _conn; });
}

template <class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> commit_impl(const Result<Ref<Connection>>& _res) {
  return _res.and_then([&](const auto& _conn) { return commit_impl(_conn); });
}

struct Commit {
  auto operator()(const auto& _conn) const { return commit_impl(_conn); }
};

inline const auto commit = Commit{};

}  // namespace sqlgen

#endif
