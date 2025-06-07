#ifndef SQLGEN_LIMIT_HPP_
#define SQLGEN_LIMIT_HPP_

#include <type_traits>

#include "Result.hpp"
#include "read.hpp"
#include "select_from.hpp"
#include "transpilation/Limit.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

using Limit = transpilation::Limit;

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType>
auto operator|(const Read<ContainerType, WhereType, OrderByType, LimitType>& _r,
               const Limit& _limit) {
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit twice.");
  return Read<ContainerType, WhereType, OrderByType, Limit>{.where_ = _r.where_,
                                                            .limit_ = _limit};
}

template <class StructType, class FieldsTupleType, class WhereType,
          class GroupByType, class OrderByType, class LimitType, class ToType>
auto operator|(
    const SelectFrom<StructType, FieldsTupleType, WhereType, GroupByType,
                     OrderByType, LimitType, ToType>& _s,
    const Limit& _limit) {
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit twice.");
  return SelectFrom<StructType, FieldsTupleType, WhereType, GroupByType,
                    OrderByType, Limit, ToType>{
      .fields_ = _s.fields_, .where_ = _s.where_, .limit_ = _limit};
}

inline auto limit(const size_t _val) { return Limit{_val}; };

}  // namespace sqlgen

#endif
