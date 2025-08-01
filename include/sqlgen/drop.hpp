#ifndef SQLGEN_DROP_HPP_
#define SQLGEN_DROP_HPP_

#include <type_traits>

#include "Ref.hpp"
#include "Result.hpp"
#include "is_connection.hpp"
#include "transpilation/to_drop.hpp"

namespace sqlgen {

template <class ValueType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> drop_impl(const Ref<Connection>& _conn,
                                  const bool _if_exists, const bool _cascade) {
  const auto query = transpilation::to_drop<ValueType>(_if_exists, _cascade);
  return _conn->execute(_conn->to_sql(query)).transform([&](const auto&) {
    return _conn;
  });
}

template <class ValueType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> drop_impl(const Result<Ref<Connection>>& _res,
                                  const bool _if_exists, const bool _cascade) {
  return _res.and_then([&](const auto& _conn) {
    return drop_impl<ValueType>(_conn, _if_exists, _cascade);
  });
}

template <class ValueType>
struct Drop {
  auto operator()(const auto& _conn) const {
    return drop_impl<ValueType>(_conn, if_exists_, cascade_);
  }

  bool cascade_ = false;
  bool if_exists_ = false;
};

template <class ContainerType>
const auto drop = Drop<ContainerType>{};

}  // namespace sqlgen

#endif
