#ifndef SQLGEN_TRANSPILATION_TO_CREATE_TABLE_HPP_
#define SQLGEN_TRANSPILATION_TO_CREATE_TABLE_HPP_

#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../dynamic/CreateTable.hpp"
#include "../dynamic/Table.hpp"
#include "get_schema.hpp"
#include "get_tablename.hpp"
#include "make_columns.hpp"

namespace sqlgen::transpilation {

template <class T>
  requires std::is_class_v<std::remove_cvref_t<T>> &&
           std::is_aggregate_v<std::remove_cvref_t<T>>
dynamic::CreateTable to_create_table(const bool _if_not_exists = true) {
  using NamedTupleType = rfl::named_tuple_t<std::remove_cvref_t<T>>;
  using Fields = typename NamedTupleType::Fields;
  return dynamic::CreateTable{
      .table =
          dynamic::Table{.name = get_tablename<T>(), .schema = get_schema<T>()},
      .columns = make_columns<Fields>(
          std::make_integer_sequence<int, rfl::tuple_size_v<Fields>>()),
      .if_not_exists = _if_not_exists};
}

}  // namespace sqlgen::transpilation

#endif
