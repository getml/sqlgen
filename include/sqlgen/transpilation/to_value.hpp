#ifndef SQLGEN_TRANSPILATION_TO_VALUE_HPP_
#define SQLGEN_TRANSPILATION_TO_VALUE_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "../dynamic/Value.hpp"
#include "Value.hpp"
#include "has_reflection_method.hpp"

namespace sqlgen::transpilation {

template <class T>
dynamic::Value to_value(const T& _t) {
  using Type = std::remove_cvref_t<T>;
  if constexpr (std::is_floating_point_v<Type>) {
    return dynamic::Value{dynamic::Float{.val = static_cast<double>(_t)}};

  } else if constexpr (std::is_integral_v<Type>) {
    return dynamic::Value{dynamic::Integer{.val = static_cast<int64_t>(_t)}};

  } else if constexpr (std::is_convertible_v<Type, std::string>) {
    return dynamic::Value{dynamic::String{.val = std::string(_t)}};

  } else if constexpr (has_reflection_method<Type>) {
    return to_value(_t.reflection());

  } else {
    static_assert(rfl::always_false_v<T>, "Unsupported type");
  }
}

}  // namespace sqlgen::transpilation

#endif
