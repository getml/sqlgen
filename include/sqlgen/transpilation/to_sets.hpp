#ifndef SQLGEN_TRANSPILATION_TO_SETS_HPP_
#define SQLGEN_TRANSPILATION_TO_SETS_HPP_

#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../Result.hpp"
#include "../col.hpp"
#include "../dynamic/Table.hpp"
#include "../dynamic/Update.hpp"
#include "Set.hpp"
#include "all_columns_exist.hpp"
#include "get_schema.hpp"
#include "get_tablename.hpp"
#include "to_condition.hpp"
#include "to_sets.hpp"
#include "to_value.hpp"

namespace sqlgen::transpilation {

template <class T, class SetType>
struct ToSet;

template <class T, rfl::internal::StringLiteral _name, class ToType>
struct ToSet<T, Set<Col<_name>, ToType>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");

  dynamic::Update::Set operator()(const auto& _set) const {
    return dynamic::Update::Set{
        .col = dynamic::Column{.name = _name.str()},
        .to = to_value(_set.to),
    };
  }
};

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToSet<T, Set<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>>(), "All columns must exist.");
  static_assert(all_columns_exist<T, Col<_name2>>(), "All columns must exist.");

  dynamic::Update::Set operator()(const auto& _set) const {
    return dynamic::Update::Set{
        .col = dynamic::Column{.name = _name1.str()},
        .to = dynamic::Column{.name = _name2.str()},
    };
  }
};

template <class T, class SetType>
dynamic::Update::Set to_set(const SetType& _set) {
  return ToSet<std::remove_cvref_t<T>, std::remove_cvref_t<SetType>>{}(_set);
}

template <class T, class SetsType>
std::vector<dynamic::Update::Set> to_sets(const SetsType& _sets) {
  return rfl::apply(
      [](const auto&... _s) {
        return std::vector<dynamic::Update::Set>({to_set<T>(_s)...});
      },
      _sets);
}

}  // namespace sqlgen::transpilation

#endif
