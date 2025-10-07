#ifndef SQLGEN_READ_HPP_
#define SQLGEN_READ_HPP_

#include <ranges>
#include <type_traits>

#include "Ref.hpp"
#include "Result.hpp"
#include "internal/is_range.hpp"
#include "is_connection.hpp"
#include "limit.hpp"
#include "order_by.hpp"
#include "transpilation/order_by_t.hpp"
#include "transpilation/read_to_select_from.hpp"
#include "transpilation/value_t.hpp"
#include "where.hpp"

namespace sqlgen {

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType, class Connection>
  requires is_connection<Connection>
auto read_impl(const Ref<Connection>& _conn, const WhereType& _where,
               const LimitType& _limit) {
  using ValueType = transpilation::value_t<ContainerType>;
  const auto query =
      transpilation::read_to_select_from<ValueType, WhereType, OrderByType,
                                         LimitType>(_where, _limit);
  return _conn->template read<ContainerType>(query);
}

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType, class Connection>
  requires is_connection<Connection>
auto read_impl(const Result<Ref<Connection>>& _res, const WhereType& _where,
               const LimitType& _limit) {
  return _res.and_then([&](const auto& _conn) {
    return read_impl<ContainerType, WhereType, OrderByType, LimitType>(
        _conn, _where, _limit);
  });
}

template <class Type, class WhereType = Nothing, class OrderByType = Nothing,
          class LimitType = Nothing>
struct Read {
  auto operator()(const auto& _conn) const {
    if constexpr (std::ranges::input_range<std::remove_cvref_t<Type>> ||
                  internal::is_range_v<Type>) {
      return read_impl<Type, WhereType, OrderByType, LimitType>(_conn, where_,
                                                                limit_);

    } else {
      return read_impl<std::vector<Type>, WhereType, OrderByType, LimitType>(
                 _conn, where_, limit_)
          .and_then([](auto&& _vec) -> Result<Type> {
            if (_vec.size() != 1) {
              return error(
                  "Because the provided type was not a container, the query "
                  "needs to return exactly one result, but it did return " +
                  std::to_string(_vec.size()) + " results.");
            }
            return std::move(_vec[0]);
          });
    }
  }

  template <class ConditionType>
  friend auto operator|(const Read& _r, const Where<ConditionType>& _where) {
    static_assert(std::is_same_v<WhereType, Nothing>,
                  "You cannot call where(...) twice (but you can apply more "
                  "than one condition by combining them with && or ||).");
    static_assert(std::is_same_v<OrderByType, Nothing>,
                  "You cannot call order_by(...) before where(...).");
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) before where(...).");
    return Read<Type, ConditionType, OrderByType, LimitType>{
        .where_ = _where.condition};
  }

  template <class... ColTypes>
  friend auto operator|(const Read& _r, const OrderBy<ColTypes...>&) {
    static_assert(
        std::is_same_v<OrderByType, Nothing>,
        "You cannot call order_by(...) twice (but you can order by more "
        "than one column).");
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) before order_by.");
    static_assert(sizeof...(ColTypes) != 0,
                  "You must assign at least one column to order by(...).");
    return Read<Type, WhereType,
                transpilation::order_by_t<
                    transpilation::value_t<Type>, Nothing,
                    typename std::remove_cvref_t<ColTypes>::ColType...>,
                LimitType>{.where_ = _r.where_};
  }

  friend auto operator|(const Read& _r, const Limit& _limit) {
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) twice.");
    return Read<Type, WhereType, OrderByType, Limit>{.where_ = _r.where_,
                                                     .limit_ = _limit};
  }

  WhereType where_;

  LimitType limit_;
};

template <class ContainerType>
const auto read = Read<ContainerType>{};

}  // namespace sqlgen

#endif
