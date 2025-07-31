#ifndef SQLGEN_CREATETABLEAS_HPP_
#define SQLGEN_CREATETABLEAS_HPP_

#include <rfl.hpp>

#include "Result.hpp"
#include "is_connection.hpp"
#include "select_from.hpp"
#include "transpilation/to_create_table_as.hpp"

namespace sqlgen {

template <class ValueType, class SelectFromType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> create_table_as_impl(const Ref<Connection>& _conn,
                                             const SelectFromType& _as,
                                             const bool _if_not_exists) {
  const auto query =
      transpilation::to_create_table_as<ValueType>(_as, _if_not_exists);
  return _conn->execute(_conn->to_sql(query)).transform([&](const auto&) {
    return _conn;
  });
}

template <class ValueType, class SelectFromType, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> create_table_as_impl(
    const Result<Ref<Connection>>& _res, const SelectFromType& _as,
    const bool _if_not_exists) {
  return _res.and_then([&](const auto& _conn) {
    return create_table_as_impl<ValueType>(_conn, _as, _if_not_exists);
  });
}

template <class ValueType, class SelectFromType>
struct CreateTableAs {
  auto operator()(const auto& _conn) const {
    return create_table_as_impl<ValueType>(_conn, as_, if_not_exists_);
  }

  SelectFromType as_;
  bool if_not_exists_;
};

template <class ValueType, class... Args>
inline auto create_table_as(const SelectFrom<Args...>& _as) {
  return CreateTableAs<std::remove_cvref_t<ValueType>, SelectFrom<Args...>>{
      .as_ = _as, .if_not_exists_ = false};
}

};  // namespace sqlgen

#endif

