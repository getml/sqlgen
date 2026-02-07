#ifndef SQLGEN_INTERNAL_HAS_AUTO_INCR_PRIMARY_KEY_HPP_
#define SQLGEN_INTERNAL_HAS_AUTO_INCR_PRIMARY_KEY_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "is_primary_key.hpp"

namespace sqlgen::internal {

template <class T, class = void>
struct is_auto_incr_primary_key : std::false_type {};

template <class T>
struct is_auto_incr_primary_key<
    T, std::void_t<decltype(std::remove_cvref_t<T>::auto_incr)>>
    : std::bool_constant<is_primary_key_v<std::remove_cvref_t<T>> &&
                         std::remove_cvref_t<T>::auto_incr> {};

template <class T>
struct has_auto_incr_primary_key;

template <class... FieldTypes>
struct has_auto_incr_primary_key<rfl::NamedTuple<FieldTypes...>> {
  constexpr static bool value =
      (false || ... ||
       is_auto_incr_primary_key<typename FieldTypes::Type>::value);
};

template <class T>
constexpr bool has_auto_incr_primary_key_v =
    has_auto_incr_primary_key<rfl::named_tuple_t<T>>::value;

}  // namespace sqlgen::internal

#endif
