#ifndef SQLGEN_TRANSPILATION_REMOVETABLEWRAPPERT_HPP_
#define SQLGEN_TRANSPILATION_REMOVETABLEWRAPPERT_HPP_

#include <type_traits>

#include "TableWrapper.hpp"

namespace sqlgen::transpilation {

template <class T>
struct RemoveTableWrapper;

template <class T>
struct RemoveTableWrapper {
  using Type = T;
};

template <class T>
struct RemoveTableWrapper<TableWrapper<T>> {
  using Type = typename RemoveTableWrapper<std::remove_cvref_t<T>>::Type;
};

template <class T>
using remove_table_wrapper_t =
    typename RemoveTableWrapper<std::remove_cvref_t<T>>::Type;

}  // namespace sqlgen::transpilation

#endif
