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
#include "../dynamic/SelectFrom.hpp"
#include "../dynamic/Table.hpp"
#include "../internal/collect/vector.hpp"
#include "get_schema.hpp"
#include "get_tablename.hpp"
#include "make_fields.hpp"
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
                                   const LimitType& _limit) {
  const auto fields = make_fields<StructType, FieldsType>(
      _fields,
      std::make_integer_sequence<int, rfl::tuple_size_v<FieldsType>>());

  return dynamic::SelectFrom{
      .table = dynamic::Table{.name = get_tablename<StructType>(),
                              .schema = get_schema<StructType>()},
      .fields = fields,
      .where = to_condition<std::remove_cvref_t<StructType>>(_where),
      .group_by = to_group_by<GroupByType>(),
      .order_by = to_order_by<OrderByType>(),
      .limit = to_limit(_limit)};
}

}  // namespace sqlgen::transpilation

#endif
