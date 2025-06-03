#ifndef SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_
#define SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "../Literal.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "As.hpp"
#include "Col.hpp"
#include "aggregations.hpp"
#include "all_columns_exist.hpp"
#include "to_value.hpp"

namespace sqlgen::transpilation {

template <class StructType, class Agg>
struct MakeField;

template <class StructType, class ValueType>
struct MakeField<StructType, ValueType> {
  static constexpr bool is_aggregation = false;

  dynamic::SelectFrom::Field operator()(const auto& _val) const {
    return dynamic::SelectFrom::Field{.val = to_value(_val)};
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct MakeField<StructType, Col<_name>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A required column does not exist.");

  static constexpr bool is_aggregation = false;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{.val =
                                          dynamic::Column{.name = _name.str()}};
  }
};

template <class StructType, class ValueType,
          class rfl::internal::StringLiteral _new_name>
struct MakeField<StructType, As<ValueType, _new_name>> {
  static constexpr bool is_aggregation =
      MakeField<StructType, ValueType>::is_aggregation;

  dynamic::SelectFrom::Field operator()(const auto& _as) const {
    return dynamic::SelectFrom::Field{
        .val =
            MakeField<StructType, std::remove_cvref_t<ValueType>>(_as.val).val,
        .as = _new_name.str()};
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct MakeField<StructType, aggregations::Avg<Col<_name>>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A column required in Avg aggregation does not exist.");

  static constexpr bool is_aggregation = true;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{
        .val = dynamic::Aggregation{dynamic::Aggregation::Avg{
            .val = dynamic::Column{.name = _name.str()}}}};
  }
};

template <class StructType, class ValueType>
struct MakeField<StructType, aggregations::Avg<ValueType>> {
  static constexpr bool is_aggregation = true;

  dynamic::SelectFrom::Field operator()(const auto& _agg) const {
    return dynamic::SelectFrom::Field{
        .val = dynamic::Aggregation{
            dynamic::Aggregation::Avg{.val = to_value(_agg.val)}}};
  }
};

template <class StructType, class ValueType>
inline dynamic::SelectFrom::Field make_field(const ValueType& _val) {
  return MakeField<std::remove_cvref_t<StructType>,
                   std::remove_cvref_t<ValueType>>{}(_val);
}

}  // namespace sqlgen::transpilation

#endif
