#ifndef SQLGEN_AS_HPP_
#define SQLGEN_AS_HPP_

#include <rfl.hpp>
#include <string>
#include <type_traits>

#include "col.hpp"
#include "transpilation/As.hpp"

namespace sqlgen {

template <rfl::internal::StringLiteral _new_name>
struct As {};

template <class ValueType, rfl::internal::StringLiteral _new_name>
auto operator|(const ValueType& _val, const As<_new_name>&) {
  return transpilation::As<std::remove_cvref_t<ValueType>, _new_name>{.val =
                                                                          _val};
}

template <rfl::internal::StringLiteral _old_name,
          rfl::internal::StringLiteral _new_name>
auto operator|(const Col<_old_name>&, const As<_new_name>&) {
  return transpilation::As<transpilation::Col<_old_name>, _new_name>{
      .val = transpilation::Col<_old_name>{}};
}

template <rfl::internal::StringLiteral _new_name>
auto operator|(const char* _val, const As<_new_name>&) {
  return transpilation::As<std::string, _new_name>{.val = _val};
}

template <rfl::internal::StringLiteral _new_name>
const auto as = As<_new_name>{};

}  // namespace sqlgen

#endif
