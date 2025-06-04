#ifndef SQLGEN_TRANSPILATION_FIELDSTONAMEDTUPLET_HPP_
#define SQLGEN_TRANSPILATION_FIELDSTONAMEDTUPLET_HPP_

#include <rfl.hpp>

#include "make_field.hpp"

namespace sqlgen::transpilation {

template <class T>
struct GetFieldName;

template <class T>
struct GetFieldName {
  static_assert(
      rfl::always_false_v<T>,
      "You must declare a field name for this field using sqlgen::as.");
};

template <rfl::internal::StringLiteral _name>
struct GetFieldName {
  static constexpr rfl::internal::StringLiteral name = _name;
};

template <class StructType, class... FieldTypes>
using fields_to_named_tuple_t = rfl::NamedTuple<rfl::Field<
    GetFieldName<typename MakeField<StructType::FieldTypes>::Name>::name,
    typename MakeField<FieldTypes>::Type>...>;

}  // namespace sqlgen::transpilation

#endif
