#ifndef SQLGEN_TRANSPILATION_TO_JOIN_HPP_
#define SQLGEN_TRANSPILATION_TO_JOIN_HPP_

#include <optional>
#include <rfl.hpp>
#include <type_traits>
#include <vector>

#include "../dynamic/Join.hpp"
#include "../dynamic/Table.hpp"
#include "Join.hpp"
#include "TableWrapper.hpp"
#include "get_schema.hpp"
#include "get_tablename.hpp"
#include "to_condition.hpp"

namespace sqlgen::transpilation {

template <class TableTupleType, class T>
struct ToJoin;

template <class TableTupleType, class TableType>
struct ToJoin<TableTupleType, TableWrapper<TableType>> {
  template <class ConditionType, rfl::internal::StringLiteral _alias>
  dynamic::Join operator()(
      const Join<TableWrapper<TableType>, ConditionType, _alias>& _join) {
    using T = std::remove_cvref_t<TableType>;
    using Alias =
        typename Join<TableWrapper<TableType>, ConditionType, _alias>::Alias;

    return dynamic::Join{
        .how = _join.how,
        .table_or_query = dynamic::Table{.name = get_tablename<T>(),
                                         .schema = get_schema<T>()},
        .alias = Alias().str(),
        .on = to_condition<TableTupleType>(_join.on)};
  }
};

template <class TableTupleType, class T, class ConditionType,
          rfl::internal::StringLiteral _alias>
dynamic::Join to_join(const Join<T, ConditionType, _alias>& _join) {
  return ToJoin<TableTupleType, T>{}(_join);
}

template <class TableTupleType, class... JoinTypes>
std::optional<std::vector<dynamic::Join>> to_joins(
    const rfl::Tuple<JoinTypes...>& _joins) {
  return rfl::apply(
      [](const auto&... _js) {
        return std::vector<dynamic::Join>({to_join<TableTupleType>(_js)...});
      },
      _joins);
}

template <class TableTupleType>
inline std::optional<std::vector<dynamic::Join>> to_joins(const Nothing&) {
  return std::nullopt;
}

}  // namespace sqlgen::transpilation

#endif
