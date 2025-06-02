#ifndef SQLGEN_GROUP_BY_HPP_
#define SQLGEN_GROUP_BY_HPP_

#include <type_traits>

#include "Result.hpp"
#include "select_from.hpp"
#include "transpilation/group_by_t.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

template <class... ColTypes>
struct GroupBy {};

template <class StructType, class FieldsTupleType, class WhereType,
          class GroupByType, class OrderByType, class LimitType,
          class... ColTypes>
auto operator|(const SelectFrom<StructType, FieldsTupleType, WhereType,
                                GroupByType, OrderByType, LimitType>& _s,
               const GroupBy<ColTypes...>&) {
  static_assert(
      std::is_same_v<GroupByType, Nothing>,
      "You cannot call group_by(...) twice (but you can group by more "
      "than one column).");
  static_assert(std::is_same_v<OrderByType, Nothing>,
                "You cannot call order_by(...) before group_by(...).");
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit(...) before group_by(...).");
  static_assert(sizeof...(ColTypes) != 0,
                "You must assign at least one column to order by.");
  return SelectFrom<StructType, FieldsTupleType, WhereType,
                    transpilation::group_by_t<StructType, ColTypes...>,
                    OrderByType, LimitType>{.where_ = _s.where_};
}

template <class... ColTypes>
auto group_by(const ColTypes&...) {
  return GroupBy<ColTypes...>{};
};

}  // namespace sqlgen

#endif
