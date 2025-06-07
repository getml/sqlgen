#ifndef SQLGEN_WHERE_HPP_
#define SQLGEN_WHERE_HPP_

#include <type_traits>

#include "Result.hpp"
#include "create_index.hpp"
#include "delete_from.hpp"
#include "read.hpp"
#include "select_from.hpp"
#include "transpilation/Limit.hpp"
#include "transpilation/value_t.hpp"
#include "update.hpp"

namespace sqlgen {

template <class ConditionType>
struct Where {
  ConditionType condition;
};

template <rfl::internal::StringLiteral _name, class ValueType, class WhereType,
          class... ColTypes, class ConditionType>
auto operator|(const CreateIndex<_name, ValueType, WhereType, ColTypes...>& _c,
               const Where<ConditionType>& _where) {
  static_assert(std::is_same_v<WhereType, Nothing>,
                "You cannot call where(...) twice (but you can apply more "
                "than one condition by combining them with && or ||).");
  return CreateIndex<_name, ValueType, ConditionType, ColTypes...>{
      .unique_ = _c.unique_,
      .if_not_exists_ = _c.if_not_exists_,
      .where_ = _where.condition};
}

template <class ValueType, class WhereType, class ConditionType>
auto operator|(const DeleteFrom<ValueType, WhereType>& _d,
               const Where<ConditionType>& _where) {
  static_assert(std::is_same_v<WhereType, Nothing>,
                "You cannot call where(...) twice (but you can apply more "
                "than one condition by combining them with && or ||).");
  return DeleteFrom<ValueType, ConditionType>{.where_ = _where.condition};
}

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType, class ConditionType>
auto operator|(const Read<ContainerType, WhereType, OrderByType, LimitType>& _r,
               const Where<ConditionType>& _where) {
  static_assert(std::is_same_v<WhereType, Nothing>,
                "You cannot call where(...) twice (but you can apply more "
                "than one condition by combining them with && or ||).");
  static_assert(std::is_same_v<OrderByType, Nothing>,
                "You cannot call order_by(...) before where(...).");
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit(...) before where(...).");
  return Read<ContainerType, ConditionType, OrderByType, LimitType>{
      .where_ = _where.condition};
}

template <class StructType, class FieldsTupleType, class WhereType,
          class GroupByType, class OrderByType, class LimitType, class ToType,
          class ConditionType>
auto operator|(
    const SelectFrom<StructType, FieldsTupleType, WhereType, GroupByType,
                     OrderByType, LimitType, ToType>& _s,
    const Where<ConditionType>& _where) {
  static_assert(std::is_same_v<WhereType, Nothing>,
                "You cannot call where(...) twice (but you can apply more "
                "than one condition by combining them with && or ||).");
  static_assert(std::is_same_v<OrderByType, Nothing>,
                "You cannot call order_by(...) before where(...).");
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit(...) before where(...).");
  static_assert(std::is_same_v<ToType, Nothing>,
                "You cannot call to<...> before where(...).");
  return SelectFrom<StructType, FieldsTupleType, ConditionType, GroupByType,
                    OrderByType, LimitType, ToType>{.fields_ = _s.fields_,
                                                    .where_ = _where.condition};
}

template <class ValueType, class SetsType, class WhereType, class ConditionType>
auto operator|(const Update<ValueType, SetsType, WhereType>& _u,
               const Where<ConditionType>& _where) {
  static_assert(std::is_same_v<WhereType, Nothing>,
                "You cannot call where(...) twice (but you can apply more "
                "than one condition by combining them with && or ||).");
  return Update<ValueType, SetsType, ConditionType>{.sets_ = _u.sets_,
                                                    .where_ = _where.condition};
}

template <class ConditionType>
inline auto where(const ConditionType& _cond) {
  return Where<std::remove_cvref_t<ConditionType>>{.condition = _cond};
};

}  // namespace sqlgen

#endif
