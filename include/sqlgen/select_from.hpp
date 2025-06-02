#ifndef SQLGEN_SELECT_FROM_HPP_
#define SQLGEN_SELECT_FROM_HPP_

#include <ranges>
#include <rfl.hpp>
#include <type_traits>

#include "Range.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "internal/is_range.hpp"
#include "is_connection.hpp"
#include "transpilation/to_select_from.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {
/*
template <class T, class ColumnsType, class WhereType, class GroupBy,
          class OrderByType, class LimitType, class Connection>
  requires is_connection<Connection>
Result<ContainerType> select_from_impl(const Ref<Connection>& _conn,
                                       class ColumnsType& _columns,
                                       const WhereType& _where,
                                       const LimitType& _limit) {
  using ValueType = transpilation::value_t<ContainerType>;
  const auto query =
      transpilation::to_select_from<ColumnsType, WhereType, OrderByType,
                                    LimitType>(_columns, _where, _limit);
  return _conn->read(query).transform(
      [](auto&& _it) { return ContainerType(_it); });
}

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType, class Connection>
  requires is_connection<Connection>
Result<ContainerType> select_from_impl(const Result<Ref<Connection>>& _res,
                                       const WhereType& _where,
                                       const LimitType& _limit) {
  return _res.and_then([&](const auto& _conn) {
    return select_from_impl<ContainerType, WhereType, OrderByType, LimitType>(
        _conn, _where, _limit);
  });
}
*/
template <class StructType, class FieldsTupleType, class WhereType = Nothing,
          class GroupByType = Nothing, class OrderByType = Nothing,
          class LimitType = Nothing>
struct SelectFrom {
  auto operator()(const auto& _conn) const {
    /*if constexpr (std::ranges::input_range<std::remove_cvref_t<Type>>) {
      return select_from_impl<Type, WhereType, OrderByType, LimitType>(
          _conn, where_, limit_);

    } else {
      const auto extract_result = [](auto&& _vec) -> Result<Type> {
        if (_vec.size() != 1) {
          return error(
              "Because the provided type was not a container, the query "
              "needs to return exactly one result, but it did return " +
              std::to_string(_vec.size()) + " results.");
        }
        return std::move(_vec[0]);
      };

      return select_from_impl<std::vector<Type>, WhereType, OrderByType,
                              LimitType>(_conn, where_, limit_)
          .and_then(extract_result);
    }*/
    return error("TODO");
  }

  FieldsTupleType fields_;

  WhereType where_;

  GroupByType group_by_;

  OrderByType order_by_;

  LimitType limit_;
};

template <class StructType, class... FieldsType>
inline select_from(const FieldsType&... _fields) {
  using FieldsTupleType = rfl::Tuple<FieldsType...>;
  return SelectFrom<StructType, FieldsTupleType>{
      .fields_ = FieldsTupleType(_fields...)};
}

}  // namespace sqlgen

#endif
