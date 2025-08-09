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
                                  const dynamic::Drop::What _what,
                                  const bool _if_exists, const bool _cascade) {
  const auto query =
      transpilation::to_drop<ValueType>(_what, _if_exists, _cascade);
  return _conn->execute(_conn->to_sql(query)).transform([&](const auto&) {
    return _conn;
  });
}

template <class ValueType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> drop_impl(const Result<Ref<Connection>>& _res,
                                  const dynamic::Drop::What _what,
                                  const bool _if_exists, const bool _cascade) {
  return _res.and_then([&](const auto& _conn) {
    return drop_impl<ValueType>(_conn, _what, _if_exists, _cascade);
  });
}

template <class ValueType>
struct Drop {
  auto operator()(const auto& _conn) const {
    return drop_impl<ValueType>(_conn, what_, if_exists_, cascade_);
  }
  dynamic::Drop::What what_;
  bool cascade_ = false;
  bool if_exists_ = false;
};

template <class ValueType>
const auto drop = Drop<ValueType>{.what_ = dynamic::Drop::What::table};

template <class ValueType>
const auto drop_table = drop<ValueType>;

template <class ValueType>
const auto drop_view = Drop<ValueType>{.what_ = dynamic::Drop::What::view};

template <class ValueType>
const auto drop_materialized_view =
    Drop<ValueType>{.what_ = dynamic::Drop::What::materialized_view};

}  // namespace sqlgen

#endif
