#ifndef SQLGEN_INTERNAL_GETCOLTYPE_HPP_
#define SQLGEN_INTERNAL_GETCOLTYPE_HPP_

#include <rfl.hpp>

#include "../col.hpp"
#include "../transpilation/Col.hpp"

namespace sqlgen::internal {

template <class T>
struct GetColType;

template <class T>
struct GetColType {
  using Type = T;
  static Type get_value(const T& _t) { return _t; }
};

template <rfl::internal::StringLiteral _name>
struct GetColType<Col<_name>> {
  using Type = transpilation::Col<_name>;
  static Type get_value(const auto&) { return transpilation::Col<_name>{}; }
};

}  // namespace sqlgen::internal

#endif
