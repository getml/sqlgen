#ifndef SQLGEN_TRANSPILATION_REMOVEREFLECTIONT_HPP_
#define SQLGEN_TRANSPILATION_REMOVEREFLECTIONT_HPP_

#include "has_reflection_method.hpp"

namespace sqlgen::transpilation {

template <class T>
struct RemoveReflection;

template <class T>
struct RemoveReflection {
  using Type = T;
};

template <class T>
  requires has_reflection_method<std::remove_cvref_t<T>>
struct RemoveReflection<T> {
  using Type = typename RemoveReflection<
      typename std::remove_cvref_t<T>::ReflectionType>::Type;
};

template <class T>
using remove_reflection_t = typename RemoveReflection<T>::Type;

}  // namespace sqlgen::transpilation

#endif
