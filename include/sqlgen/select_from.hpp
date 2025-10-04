#ifndef SQLGEN_SELECT_FROM_HPP_
#define SQLGEN_SELECT_FROM_HPP_

#include <ranges>
#include <rfl.hpp>
#include <type_traits>

#include "Range.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "col.hpp"
#include "dynamic/Join.hpp"
#include "dynamic/SelectFrom.hpp"
#include "group_by.hpp"
#include "internal/GetColType.hpp"
#include "internal/is_range.hpp"
#include "is_connection.hpp"
#include "limit.hpp"
#include "order_by.hpp"
#include "to.hpp"
#include "transpilation/Join.hpp"
#include "transpilation/TableWrapper.hpp"
#include "transpilation/fields_to_named_tuple_t.hpp"
#include "transpilation/group_by_t.hpp"
#include "transpilation/order_by_t.hpp"
#include "transpilation/table_tuple_t.hpp"
#include "transpilation/to_joins.hpp"
#include "transpilation/to_select_from.hpp"
#include "transpilation/value_t.hpp"
#include "where.hpp"

namespace sqlgen {

template <class TableTupleType, class AliasType, class FieldsType,
          class TableOrQueryType, class JoinsType, class WhereType,
          class GroupByType, class OrderByType, class LimitType,
          class ContainerType, class Connection>
  requires is_connection<Connection>
auto select_from_impl(const Ref<Connection>& _conn, const FieldsType& _fields,
                      const TableOrQueryType& _table_or_query,
                      const JoinsType& _joins, const WhereType& _where,
                      const LimitType& _limit) {
  if constexpr (internal::is_range_v<ContainerType>) {
    const auto query =
        transpilation::to_select_from<TableTupleType, AliasType, FieldsType,
                                      TableOrQueryType, JoinsType, WhereType,
                                      GroupByType, OrderByType, LimitType>(
            _fields, _table_or_query, _joins, _where, _limit);
    return _conn->template read<ContainerType>(query);

  } else {
    const auto to_container = [](auto range) -> Result<ContainerType> {
      using ValueType = transpilation::value_t<ContainerType>;
      ContainerType container;
      for (auto& res : range) {
        if (res) {
          container.emplace_back(
              rfl::from_named_tuple<ValueType>(std::move(*res)));
        } else {
          return error(res.error().what());
        }
      }
      return container;
    };

    using RangeType = Range<
        transpilation::fields_to_named_tuple_t<TableTupleType, FieldsType>>;

    return select_from_impl<TableTupleType, AliasType, FieldsType,
                            TableOrQueryType, JoinsType, WhereType, GroupByType,
                            OrderByType, LimitType, RangeType>(
               _conn, _fields, _table_or_query, _joins, _where, _limit)
        .and_then(to_container);
  }
}

template <class TableTupleType, class AliasType, class FieldsType,
          class TableOrQueryType, class JoinsType, class WhereType,
          class GroupByType, class OrderByType, class LimitType,
          class ContainerType, class Connection>
  requires is_connection<Connection>
auto select_from_impl(const Result<Ref<Connection>>& _res,
                      const FieldsType& _fields,
                      const TableOrQueryType& _table_or_query,
                      const JoinsType& _joins, const WhereType& _where,
                      const LimitType& _limit) {
  return _res.and_then([&](const auto& _conn) {
    return select_from_impl<TableTupleType, AliasType, FieldsType,
                            TableOrQueryType, JoinsType, WhereType, GroupByType,
                            OrderByType, LimitType, ContainerType>(
        _conn, _fields, _table_or_query, _joins, _where, _limit);
  });
}

template <class TableOrQueryType, class AliasType, class FieldsType,
          class JoinsType = Nothing, class WhereType = Nothing,
          class GroupByType = Nothing, class OrderByType = Nothing,
          class LimitType = Nothing, class ToType = Nothing>
struct SelectFrom {
  auto operator()(const auto& _conn) const {
    using TableTupleType =
        transpilation::table_tuple_t<TableOrQueryType, AliasType, JoinsType>;

    if constexpr (std::is_same_v<ToType, Nothing> ||
                  std::ranges::input_range<std::remove_cvref_t<ToType>>) {
      using ContainerType =
          std::conditional_t<std::is_same_v<ToType, Nothing>,
                             Range<transpilation::fields_to_named_tuple_t<
                                 TableTupleType, FieldsType>>,
                             ToType>;
      return select_from_impl<
          TableTupleType, AliasType, FieldsType, TableOrQueryType, JoinsType,
          WhereType, GroupByType, OrderByType, LimitType, ContainerType>(
          _conn, fields_, from_, joins_, where_, limit_);

    } else {
      const auto extract_result = [](auto&& _vec) -> Result<ToType> {
        if (_vec.size() != 1) {
          return error(
              "Because the type provided to to<...> was not a container, the "
              "query needs to return exactly one result, but it did return " +
              std::to_string(_vec.size()) + " results.");
        }
        return std::move(_vec[0]);
      };

      return select_from_impl<TableTupleType, AliasType, FieldsType,
                              TableOrQueryType, JoinsType, WhereType,
                              GroupByType, OrderByType, LimitType,
                              std::vector<std::remove_cvref_t<ToType>>>(
                 _conn, fields_, from_, joins_, where_, limit_)
          .and_then(extract_result);
    }
  }

  template <class OtherTableOrQueryType, class ConditionType,
            rfl::internal::StringLiteral _alias, transpilation::JoinType _how>
  friend auto operator|(
      const SelectFrom& _s,
      const transpilation::Join<OtherTableOrQueryType, ConditionType, _alias,
                                _how>& _join) {
    static_assert(std::is_same_v<WhereType, Nothing>,
                  "You cannot call where(...) before a join.");
    static_assert(std::is_same_v<GroupByType, Nothing>,
                  "You cannot call group_by(...) before a join.");
    static_assert(std::is_same_v<OrderByType, Nothing>,
                  "You cannot call order_by(...) before a join.");
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) before a join.");
    static_assert(std::is_same_v<ToType, Nothing>,
                  "You cannot call to<...> before a join.");

    if constexpr (std::is_same_v<JoinsType, Nothing>) {
      using NewJoinsType =
          rfl::Tuple<transpilation::Join<OtherTableOrQueryType, ConditionType,
                                         _alias, _how>>;

      return SelectFrom<TableOrQueryType, AliasType, FieldsType, NewJoinsType,
                        WhereType, GroupByType, OrderByType, LimitType, ToType>{
          .fields_ = _s.fields_,
          .from_ = _s.from_,
          .joins_ = NewJoinsType(_join)};

    } else {
      using TupleType =
          rfl::Tuple<transpilation::Join<OtherTableOrQueryType, ConditionType,
                                         _alias, _how>>;

      const auto joins = rfl::tuple_cat(_s.joins_, TupleType(_join));

      using NewJoinsType = std::remove_cvref_t<decltype(joins)>;

      return SelectFrom<TableOrQueryType, AliasType, FieldsType, NewJoinsType,
                        WhereType, GroupByType, OrderByType, LimitType, ToType>{
          .fields_ = _s.fields_, .from_ = _s.from_, .joins_ = joins};
    }
  }

  template <class ConditionType>
  friend auto operator|(const SelectFrom& _s,
                        const Where<ConditionType>& _where) {
    static_assert(std::is_same_v<WhereType, Nothing>,
                  "You cannot call where(...) twice (but you can apply more "
                  "than one condition by combining them with && or ||).");
    static_assert(std::is_same_v<GroupByType, Nothing>,
                  "You cannot call group_by(...) before where(...).");
    static_assert(std::is_same_v<OrderByType, Nothing>,
                  "You cannot call order_by(...) before where(...).");
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) before where(...).");
    static_assert(std::is_same_v<ToType, Nothing>,
                  "You cannot call to<...> before where(...).");
    return SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType,
                      ConditionType, GroupByType, OrderByType, LimitType,
                      ToType>{.fields_ = _s.fields_,
                              .from_ = _s.from_,
                              .joins_ = _s.joins_,
                              .where_ = _where.condition};
  }

  template <class... ColTypes>
  friend auto operator|(const SelectFrom& _s, const GroupBy<ColTypes...>&) {
    static_assert(
        std::is_same_v<GroupByType, Nothing>,
        "You cannot call group_by(...) twice (but you can group by more "
        "than one column).");
    static_assert(std::is_same_v<OrderByType, Nothing>,
                  "You cannot call order_by(...) before group_by(...).");
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) before group_by(...).");
    static_assert(std::is_same_v<ToType, Nothing>,
                  "You cannot call to<...> before group_by(...).");
    static_assert(sizeof...(ColTypes) != 0,
                  "You must assign at least one column to group_by.");
    return SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType,
                      WhereType,
                      transpilation::group_by_t<TableOrQueryType,
                                                typename ColTypes::ColType...>,
                      OrderByType, LimitType, ToType>{.fields_ = _s.fields_,
                                                      .from_ = _s.from_,
                                                      .joins_ = _s.joins_,
                                                      .where_ = _s.where_};
  }

  template <class... ColTypes>
  friend auto operator|(const SelectFrom& _s, const OrderBy<ColTypes...>&) {
    static_assert(
        std::is_same_v<OrderByType, Nothing>,
        "You cannot call order_by(...) twice (but you can order by more "
        "than one column).");
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit(...) before order_by(...).");
    static_assert(std::is_same_v<ToType, Nothing>,
                  "You cannot call to<...> before order_by(...).");
    static_assert(sizeof...(ColTypes) != 0,
                  "You must assign at least one column to order_by.");

    using TableTupleType =
        transpilation::table_tuple_t<TableOrQueryType, AliasType, JoinsType>;

    using NewOrderByType = transpilation::order_by_t<
        TableTupleType, GroupByType,
        typename std::remove_cvref_t<ColTypes>::ColType...>;

    return SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType,
                      WhereType, GroupByType, NewOrderByType, LimitType,
                      ToType>{.fields_ = _s.fields_,
                              .from_ = _s.from_,
                              .joins_ = _s.joins_,
                              .where_ = _s.where_};
  }

  friend auto operator|(const SelectFrom& _s, const Limit& _limit) {
    static_assert(std::is_same_v<LimitType, Nothing>,
                  "You cannot call limit twice.");
    return SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType,
                      WhereType, GroupByType, OrderByType, Limit, ToType>{
        .fields_ = _s.fields_,
        .from_ = _s.from_,
        .joins_ = _s.joins_,
        .where_ = _s.where_,
        .limit_ = _limit};
  }

  template <class NewToType>
  friend auto operator|(const SelectFrom& _s, const To<NewToType>&) {
    static_assert(std::is_same_v<ToType, Nothing>,
                  "You cannot call to<...> twice.");
    return SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType,
                      WhereType, GroupByType, OrderByType, LimitType,
                      NewToType>{.fields_ = _s.fields_,
                                 .from_ = _s.from_,
                                 .joins_ = _s.joins_,
                                 .where_ = _s.where_,
                                 .limit_ = _s.limit_};
  }

  FieldsType fields_;

  TableOrQueryType from_;

  JoinsType joins_;

  WhereType where_;

  LimitType limit_;
};

namespace transpilation {

template <class TableOrQueryType, class AliasType, class FieldsType,
          class JoinsType, class... Args>
struct ExtractTable<
    SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType, Args...>,
    false> {
  using TableTupleType = table_tuple_t<TableOrQueryType, AliasType, JoinsType>;
  using Type = fields_to_named_tuple_t<TableTupleType, FieldsType>;
};

template <class TableOrQueryType, class AliasType, class FieldsType,
          class JoinsType, class WhereType, class GroupByType,
          class OrderByType, class LimitType, class ToType>
struct ToTableOrQuery<
    SelectFrom<TableOrQueryType, AliasType, FieldsType, JoinsType, WhereType,
               GroupByType, OrderByType, LimitType, ToType>> {
  dynamic::SelectFrom::TableOrQueryType operator()(const auto& _query) {
    using TableTupleType =
        table_tuple_t<TableOrQueryType, AliasType, JoinsType>;
    return Ref<dynamic::SelectFrom>::make(
        transpilation::to_select_from<TableTupleType, AliasType, FieldsType,
                                      TableOrQueryType, JoinsType, WhereType,
                                      GroupByType, OrderByType, LimitType>(
            _query.fields_, _query.from_, _query.joins_, _query.where_,
            _query.limit_));
  }
};

}  // namespace transpilation

template <class TableType, class... FieldTypes>
inline auto select_from(const FieldTypes&... _fields) {
  using FieldsType =
      rfl::Tuple<typename internal::GetColType<FieldTypes>::Type...>;
  return SelectFrom<transpilation::TableWrapper<TableType>, Nothing,
                    FieldsType>{
      .fields_ =
          FieldsType(internal::GetColType<FieldTypes>::get_value(_fields)...),
      .from_ = transpilation::TableWrapper<TableType>{}};
}

template <class TableType, rfl::internal::StringLiteral _alias,
          class... FieldTypes>
inline auto select_from(const FieldTypes&... _fields) {
  using FieldsType =
      rfl::Tuple<typename internal::GetColType<FieldTypes>::Type...>;
  return SelectFrom<transpilation::TableWrapper<TableType>, Literal<_alias>,
                    FieldsType>{
      .fields_ =
          FieldsType(internal::GetColType<FieldTypes>::get_value(_fields)...),
      .from_ = transpilation::TableWrapper<TableType>{}};
}

template <rfl::internal::StringLiteral _alias, class QueryType,
          class... FieldTypes>
inline auto select_from(const QueryType& _query, const FieldTypes&... _fields) {
  using FieldsType =
      rfl::Tuple<typename internal::GetColType<FieldTypes>::Type...>;
  return SelectFrom<QueryType, Literal<_alias>, FieldsType>{
      .fields_ =
          FieldsType(internal::GetColType<FieldTypes>::get_value(_fields)...),
      .from_ = _query};
}

}  // namespace sqlgen

#endif
