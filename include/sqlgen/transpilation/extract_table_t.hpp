#ifndef SQLGEN_TRANSPILATION_EXTRACTTABLET_HPP_
#define SQLGEN_TRANSPILATION_EXTRACTTABLET_HPP_

#include <type_traits>

#include "TableWrapper.hpp"

namespace sqlgen::transpilation {

template <class T>
struct ExtractTable;

template <class T>
struct ExtractTable {
  using Type = T;
};

template <class T>
struct ExtractTable<TableWrapper<T>> {
  using Type = typename ExtractTable<std::remove_cvref_t<T>>::Type;
};

template <class T>
using extract_table_t = typename ExtractTable<std::remove_cvref_t<T>>::Type;

}  // namespace sqlgen::transpilation

#endif
