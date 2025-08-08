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
#include "to_joins.hpp"
#include "to_limit.hpp"
#include "to_order_by.hpp"

namespace sqlgen::transpilation {

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

  const auto fields = make_fields<TableTupleType, FieldsType>(
      _fields,
      std::make_integer_sequence<int, rfl::tuple_size_v<FieldsType>>());

  return dynamic::SelectFrom{
      .table_or_query = dynamic::Table{.alias = to_alias<AliasType>(),
                                       .name = get_tablename<StructType>(),
                                       .schema = get_schema<StructType>()},
      .fields = fields,
      .alias = to_alias<AliasType>(),
      .joins = to_joins<TableTupleType>(_joins),
      .where = to_condition<std::remove_cvref_t<TableTupleType>>(_where),
      .group_by = to_group_by<GroupByType>(),
      .order_by = to_order_by<OrderByType>(),
      .limit = to_limit(_limit)};
}

}  // namespace sqlgen::transpilation

#endif
