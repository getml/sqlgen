#ifndef SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_
#define SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "../Literal.hpp"
#include "../Result.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "As.hpp"
#include "Col.hpp"
#include "Value.hpp"
#include "aggregations.hpp"
#include "all_columns_exist.hpp"
#include "to_value.hpp"

namespace sqlgen::transpilation {

template <class StructType, class Agg>
struct MakeField;

template <class StructType, class ValueType>
struct MakeField {
  static constexpr bool is_aggregation = false;

  using Name = Nothing;
  using Type = ValueType;

  dynamic::SelectFrom::Field operator()(const auto& _val) const {
    return dynamic::SelectFrom::Field{.val = to_value(_val)};
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct MakeField<StructType, Col<_name>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A required column does not exist.");

  static constexpr bool is_aggregation = false;

  using Name = Literal<_name>;
  using Type = rfl::field_type_t<_name, StructType>;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{.val =
                                          dynamic::Column{.name = _name.str()}};
  }
};

template <class StructType, class ValueType,
          rfl::internal::StringLiteral _new_name>
struct MakeField<StructType, As<ValueType, _new_name>> {
  static constexpr bool is_aggregation =
      MakeField<StructType, ValueType>::is_aggregation;

  using Name = Literal<_new_name>;
  using Type =
      typename MakeField<StructType, std::remove_cvref_t<ValueType>>::Type;

  dynamic::SelectFrom::Field operator()(const auto& _as) const {
    return dynamic::SelectFrom::Field{
        .val = MakeField<StructType, std::remove_cvref_t<ValueType>>{}(_as.val)
                   .val,
        .as = _new_name.str()};
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct MakeField<StructType, aggregations::Avg<Col<_name>>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A column required in Avg aggregation does not exist.");

  static constexpr bool is_aggregation = true;

  using Name = Literal<_name>;
  using Type = rfl::field_type_t<_name, StructType>;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{
        .val = dynamic::Aggregation{dynamic::Aggregation::Avg{
            .val = dynamic::Column{.name = _name.str()}}}};
  }
};

template <class StructType, class ValueType>
inline dynamic::SelectFrom::Field make_field(const ValueType& _val) {
  return MakeField<std::remove_cvref_t<StructType>,
                   std::remove_cvref_t<ValueType>>{}(_val);
}

}  // namespace sqlgen::transpilation

#endif
