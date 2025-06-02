#ifndef SQLGEN_ORDER_BY_HPP_
#define SQLGEN_ORDER_BY_HPP_

#include <type_traits>

#include "Result.hpp"
#include "read.hpp"
#include "transpilation/order_by_t.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

template <class... ColTypes>
struct OrderBy {};

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType, class... ColTypes>
auto operator|(const Read<ContainerType, WhereType, OrderByType, LimitType>& _r,
               const OrderBy<ColTypes...>&) {
  static_assert(std::is_same_v<OrderByType, Nothing>,
                "You cannot call order_by twice (but you can order by more "
                "than one column).");
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit(...) before order_by.");
  static_assert(sizeof...(ColTypes) != 0,
                "You must assign at least one column to order by.");
  return Read<ContainerType, WhereType,
              transpilation::order_by_t<
                  transpilation::value_t<ContainerType>,
                  typename std::remove_cvref_t<ColTypes>::ColType...>,
              LimitType>{.where_ = _r.where_};
}

template <class... ColTypes>
auto order_by(const ColTypes&...) {
  return OrderBy<ColTypes...>{};
};

}  // namespace sqlgen

#endif
