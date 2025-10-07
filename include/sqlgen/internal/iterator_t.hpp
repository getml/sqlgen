#ifndef SQLGEN_TRANSPILATION_ITERATOR_T_HPP_
#define SQLGEN_TRANSPILATION_ITERATOR_T_HPP_

#include <type_traits>

#include "../Iterator.hpp"

namespace sqlgen::internal {

template <class ValueType, class ConnType>
struct IteratorType;

/// Most database connectors can just use the standard iterator type, but
/// sometimes we need exception, in which case this template can be overridden.
template <class ValueType, class ConnType>
struct IteratorType {
  using Type = Iterator<ValueType>;
};

template <class T, class ConnType>
using iterator_t = typename IteratorType<std::remove_cvref_t<T>,
                                         std::remove_cvref_t<ConnType>>::Type;

}  // namespace sqlgen::internal

#endif
