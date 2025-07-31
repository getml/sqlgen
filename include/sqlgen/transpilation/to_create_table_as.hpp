#ifndef SQLGEN_TRANSPILATION_TO_CREATE_TABLE_AS_HPP_
#define SQLGEN_TRANSPILATION_TO_CREATE_TABLE_AS_HPP_

#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../dynamic/CreateTableAs.hpp"
#include "../dynamic/Table.hpp"
#include "get_schema.hpp"
#include "get_tablename.hpp"
#include "to_select_from.hpp"

namespace sqlgen::transpilation {

template <class T, class TableTupleType, class AliasType, class FieldsType,
          class JoinsType, class WhereType, class GroupByType,
          class OrderByType, class LimitType>
  requires std::is_class_v<std::remove_cvref_t<T>> &&
           std::is_aggregate_v<std::remove_cvref_t<T>>
dynamic::CreateTableAs to_create_table_as(const FieldsType& _fields,
                                          const JoinsType& _joins,
                                          const WhereType& _where,
                                          const LimitType& _limit,
                                          const bool _if_not_exists) {
  return dynamic::CreateTableAs{
      .table =
          dynamic::Table{.name = get_tablename<T>(), .schema = get_schema<T>()},
      .as = to_select_from<TableTupleType, AliasType, FieldsType, JoinsType,
                           WhereType, GroupByType, OrderByType, LimitType>(
          _as.fields_, _as.joins_, _as.where_, _as.limit_),
      .if_not_exists = _if_not_exists};
}

}  // namespace sqlgen::transpilation

#endif
