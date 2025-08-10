#ifndef SQLGEN_TRANSPILATION_GET_TABLE_OR_VIEW_HPP_
#define SQLGEN_TRANSPILATION_GET_TABLE_OR_VIEW_HPP_

#include <concepts>
#include <type_traits>

#include "../dynamic/TableOrView.hpp"

namespace sqlgen::transpilation {

template <class Type>
consteval dynamic::TableOrView get_table_or_view() {
  using T = std::remove_cvref_t<Type>;

  constexpr bool has_materialized_view = requires {
    { T::is_materialized_view } -> std::convertible_to<bool>;
  };

  constexpr bool has_view = requires {
    { T::is_view } -> std::convertible_to<bool>;
  };

  if constexpr (has_materialized_view) {
    static_assert(!has_view,
                  "A struct cannot have both the is_materialized_view and "
                  "is_view marker (even if one of them is set to false).");
    if constexpr (T::is_materialized_view) {
      return dynamic::TableOrView::materialized_view;
    }
  } else if constexpr (has_view) {
    if constexpr (T::is_view) {
      return dynamic::TableOrView::view;
    }
  } else {
    return dynamic::TableOrView::table;
  }
}

}  // namespace sqlgen::transpilation

#endif
