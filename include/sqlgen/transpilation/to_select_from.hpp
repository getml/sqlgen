#ifndef SQLGEN_TRANSPILATION_TO_SELECT_FROM_HPP_
#define SQLGEN_TRANSPILATION_TO_SELECT_FROM_HPP_

#include <ranges>
#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../Result.hpp"
#include "../dynamic/ColumnOrAggregation.hpp"
#include "../dynamic/Join.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "../dynamic/Table.hpp"
#include "../internal/collect/vector.hpp"
#include "Join.hpp"
#include "check_aggregations.hpp"
#include "flatten_fields_t.hpp"
#include "get_schema.hpp"
#include "get_table_t.hpp"
#include "get_tablename.hpp"
#include "make_fields.hpp"
#include "to_alias.hpp"
#include "to_condition.hpp"
#include "to_group_by.hpp"
#include "to_limit.hpp"
#include "to_order_by.hpp"

namespace sqlgen::transpilation {

template <class StructType, class FieldsType, class WhereType,
          class GroupByType, class OrderByType, class LimitType>
  requires std::is_class_v<std::remove_cvref_t<StructType>> &&
           std::is_aggregate_v<std::remove_cvref_t<StructType>>
dynamic::SelectFrom to_select_from(const FieldsType& _fields,
                                   const WhereType& _where,
                                   const LimitType& _limit);

template <class TableType, class ConditionType,
          rfl::internal::StringLiteral _alias>
  requires std::is_class_v<std::remove_cvref_t<TableType>> &&
           std::is_aggregate_v<std::remove_cvref_t<TableType>>
dynamic::Join to_join(
    const Join<TableWrapper<TableType>, ConditionType, _alias>& _join) {
  using T = std::remove_cvref_t<TableType>;
  using Alias =
      typename Join<TableWrapper<TableType>, ConditionType, _alias>::Alias;

  return dynamic::Join{
      .how = _join.how,
      .table_or_query =
          dynamic::Table{.name = get_tablename<T>(), .schema = get_schema<T>()},
      .alias = Alias().str()};
}

template <class... JoinTypes>
std::optional<std::vector<dynamic::Join>> to_joins(
    const rfl::Tuple<JoinTypes...>& _joins) {
  return rfl::apply(
      [](const auto&... _js) {
        return std::vector<dynamic::Join>({to_join(_js)...});
      },
      _joins);
}

inline std::optional<std::vector<dynamic::Join>> to_joins(const Nothing&) {
  return std::nullopt;
}

template <class TableTupleType, class AliasType, class FieldsType,
          class JoinsType, class WhereType, class GroupByType,
          class OrderByType, class LimitType>
dynamic::SelectFrom to_select_from(const FieldsType& _fields,
                                   const JoinsType& _joins,
                                   const WhereType& _where,
                                   const LimitType& _limit) {
  static_assert(check_aggregations<TableTupleType,
                                   flatten_fields_t<TableTupleType, FieldsType>,
                                   GroupByType>(),
                "The aggregations were not set up correctly. Please check the "
                "trace for a more detailed error message.");

  using StructType =
      get_table_t<std::integral_constant<size_t, 0>, TableTupleType>;

  const auto fields = make_fields<StructType, FieldsType>(
      _fields,
      std::make_integer_sequence<int, rfl::tuple_size_v<FieldsType>>());

  return dynamic::SelectFrom{
      .table = dynamic::Table{.alias = to_alias<AliasType>(),
                              .name = get_tablename<StructType>(),
                              .schema = get_schema<StructType>()},
      .fields = fields,
      .joins = to_joins(_joins),
      .where = to_condition<std::remove_cvref_t<TableTupleType>>(_where),
      .group_by = to_group_by<GroupByType>(),
      .order_by = to_order_by<OrderByType>(),
      .limit = to_limit(_limit)};
}

}  // namespace sqlgen::transpilation

#endif
