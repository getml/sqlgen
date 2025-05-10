#ifndef SQLGEN_DELETE_FROM_HPP_
#define SQLGEN_DELETE_FROM_HPP_

#include <type_traits>

#include "Connection.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "transpilation/to_delete_from.hpp"

namespace sqlgen {

template <class ValueType, class WhereType>
Result<Nothing> delete_from_impl(const Ref<Connection>& _conn,
                                 const WhereType& _where) {
  const auto query =
      transpilation::to_delete_from<ValueType, WhereType>(_where);
  return _conn->execute(_conn->to_sql(query));
}

template <class ValueType, class WhereType>
Result<Nothing> delete_from_impl(const Result<Ref<Connection>>& _res,
                                 const WhereType& _where) {
  return _res.and_then([&](const auto& _conn) {
    return delete_from_impl<ValueType, WhereType>(_conn, _where);
  });
}

template <class ValueType, class WhereType = Nothing>
struct DeleteFrom {
  Result<Nothing> operator()(const auto& _conn) const noexcept {
    try {
      return delete_from_impl<ValueType, WhereType>(_conn, where_);
    } catch (std::exception& e) {
      return error(e.what());
    }
  }

  WhereType where_;
};

template <class ContainerType>
const auto delete_from = DeleteFrom<ContainerType>{};

}  // namespace sqlgen

#endif
