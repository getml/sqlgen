#ifndef SQLGEN_TRANSPILATION_ALLCOLUMNSEXIST_HPP_
#define SQLGEN_TRANSPILATION_ALLCOLUMNSEXIST_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "../Literal.hpp"
#include "Col.hpp"

namespace sqlgen::transpilation {

template <class Names, class ColType>
struct ColumnExists;

template <rfl::internal::StringLiteral _col_name,
          rfl::internal::StringLiteral... _field_names>
struct ColumnExists<Literal<_field_names...>, transpilation::Col<_col_name>> {
  static constexpr bool value = (false || ... || (_col_name == _field_names));
  static_assert(value, "Column does not exist.");
};

template <class Names, class ColType>
constexpr bool column_exists_v = ColumnExists<Names, ColType>::value;

template <class T, class... ColTypes>
consteval bool all_columns_exist() {
  using Names = typename rfl::named_tuple_t<std::remove_cvref_t<T>>::Names;
  return (true && ... && column_exists_v<Names, ColTypes>);
}

}  // namespace sqlgen::transpilation

#endif
