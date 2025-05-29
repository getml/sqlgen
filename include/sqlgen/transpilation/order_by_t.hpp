#ifndef SQLGEN_TRANSPILATION_ORDERBYT_HPP_
#define SQLGEN_TRANSPILATION_ORDERBYT_HPP_

#include <rfl.hpp>
#include <type_traits>
#include <vector>

#include "Col.hpp"
#include "Desc.hpp"
#include "all_columns_exist.hpp"

namespace sqlgen::transpilation {

template <class _ColType>
struct OrderByWrapper;

template <rfl::internal::StringLiteral _name>
struct OrderByWrapper<transpilation::Col<_name>> {
  using ColType = transpilation::Col<_name>;
  constexpr static bool desc = false;
};

template <rfl::internal::StringLiteral _name>
struct OrderByWrapper<Desc<transpilation::Col<_name>>> {
  using ColType = transpilation::Col<_name>;
  constexpr static bool desc = true;
};

template <class... WrapperTypes>
struct OrderBy {};

template <class T, class... ColTypes>
auto make_order_by() {
  static_assert(
      all_columns_exist<T, typename OrderByWrapper<ColTypes>::ColType...>(),
      "All columns must exist.");
  return OrderBy<OrderByWrapper<ColTypes>...>{};
}

template <class T, class... ColTypes>
using order_by_t = std::invoke_result_t<
    decltype(make_order_by<T, typename ColTypes::ColType...>)>;

}  // namespace sqlgen::transpilation

#endif
