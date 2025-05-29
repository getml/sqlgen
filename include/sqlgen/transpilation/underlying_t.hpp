#ifndef SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_
#define SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "Col.hpp"
#include "Desc.hpp"
#include "Value.hpp"
#include "remove_reflection_t.hpp"

namespace sqlgen::transpilation {

template <class T, class _Type>
struct Underlying;

template <class T, rfl::internal::StringLiteral _name>
struct Underlying<T, Col<_name>> {
  using Type = remove_reflection_t<rfl::field_type_t<_name, T>>;
};

template <class T, rfl::internal::StringLiteral _name>
struct Underlying<T, Desc<Col<_name>>> {
  using Type = remove_reflection_t<rfl::field_type_t<_name, T>>;
};

template <class T, class _Type>
struct Underlying<T, Value<_Type>> {
  using Type = _Type;
};

template <class T, class U>
using underlying_t =
    typename Underlying<std::remove_cvref_t<T>, std::remove_cvref_t<U>>::Type;

}  // namespace sqlgen::transpilation

#endif
