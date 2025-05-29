#ifndef SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_
#define SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_

#include <type_traits>

#include "Value.hpp"

namespace sqlgen::transpilation {

template <class T, class _Type>
struct Underlying;

template <class T, class _Type>
struct Underlying<T, Value<_Type>> {
  using Type = _Type;
};

template <class T>
using underlying_t = typename Underlying<std::remove_cvref_t<T>>::Type;

}  // namespace sqlgen::transpilation

#endif
