#ifndef SQLGEN_UPDATE_HPP_
#define SQLGEN_UPDATE_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "Ref.hpp"
#include "Result.hpp"
#include "is_connection.hpp"
#include "transpilation/to_update.hpp"

namespace sqlgen {

template <class ValueType, class SetsType, class WhereType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> update_impl(const Ref<Connection>& _conn,
                                    const SetsType& _sets,
                                    const WhereType& _where) {
  const auto query =
      transpilation::to_update<ValueType, SetsType, WhereType>(_sets, _where);
  return _conn->execute(_conn->to_sql(query)).transform([&](const auto&) {
    return _conn;
  });
}

template <class ValueType, class SetsType, class WhereType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> update_impl(const Result<Ref<Connection>>& _res,
                                    const SetsType& _sets,
                                    const WhereType& _where) {
  return _res.and_then([&](const auto& _conn) {
    return update_impl<ValueType, SetsType, WhereType>(_conn, _sets, _where);
  });
}

template <class ValueType, class SetsType, class WhereType = Nothing>
struct Update {
  auto operator()(const auto& _conn) const {
    return update_impl<ValueType, SetsType, WhereType>(_conn, sets_, where_);
  }

  SetsType sets_;

  WhereType where_;
};

template <class ValueType, class... SetsType>
auto update(const SetsType&... _sets) {
  static_assert(sizeof...(_sets) > 0, "You must update at least one column.");
  using TupleType = rfl::Tuple<std::remove_cvref_t<SetsType>...>;
  return Update<ValueType, TupleType>{.sets_ = TupleType(_sets...)};
};

}  // namespace sqlgen

#endif
