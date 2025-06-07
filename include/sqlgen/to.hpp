#ifndef SQLGEN_TO_HPP_
#define SQLGEN_TO_HPP_

#include <type_traits>

#include "Result.hpp"
#include "select_from.hpp"

namespace sqlgen {

template <class NewToType>
struct To {};

template <class StructType, class FieldsTupleType, class WhereType,
          class GroupByType, class OrderByType, class LimitType, class ToType,
          class NewToType>
auto operator|(
    const SelectFrom<StructType, FieldsTupleType, WhereType, GroupByType,
                     OrderByType, LimitType, ToType>& _s,
    const To<NewToType>&) {
  static_assert(std::is_same_v<GroupByType, Nothing>,
                "You cannot call to<...> twice.");
  return SelectFrom<StructType, FieldsTupleType, WhereType, GroupByType,
                    OrderByType, LimitType, NewToType>{
      .fields_ = _s.fields_, .where_ = _s.where_, .limit_ = _s.limit_};
}

template <class NewToType>
const auto to = To<NewToType>{};

}  // namespace sqlgen

#endif
