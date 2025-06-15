#ifndef SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_
#define SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "../Literal.hpp"
#include "../Result.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "Aggregation.hpp"
#include "AggregationOp.hpp"
#include "As.hpp"
#include "Col.hpp"
#include "Operation.hpp"
#include "Operator.hpp"
#include "Value.hpp"
#include "all_columns_exist.hpp"
#include "dynamic_aggregation_t.hpp"
#include "dynamic_operator_t.hpp"
#include "remove_nullable_t.hpp"
#include "to_value.hpp"
#include "underlying_t.hpp"

namespace sqlgen::transpilation {

template <class StructType, class FieldType>
struct MakeField;

template <class StructType, class ValueType>
struct MakeField {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;

  using Name = Nothing;
  using Type = ValueType;

  dynamic::SelectFrom::Field operator()(const auto& _val) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{.val = to_value(_val)}};
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct MakeField<StructType, Col<_name>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A required column does not exist.");

  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = true;

  using Name = Literal<_name>;
  using Type = rfl::field_type_t<_name, StructType>;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{.val = dynamic::Column{.name = _name.str()}}};
  }
};

template <class StructType, class ValueType,
          rfl::internal::StringLiteral _new_name>
struct MakeField<StructType, As<ValueType, _new_name>> {
  static constexpr bool is_aggregation =
      MakeField<StructType, ValueType>::is_aggregation;
  static constexpr bool is_column = MakeField<StructType, ValueType>::is_column;

  using Name = Literal<_new_name>;
  using Type =
      typename MakeField<StructType, std::remove_cvref_t<ValueType>>::Type;

  dynamic::SelectFrom::Field operator()(const auto& _as) const {
    return dynamic::SelectFrom::Field{
        .val =
            dynamic::Operation{
                .val = MakeField<StructType, std::remove_cvref_t<ValueType>>{}(
                           _as.val)
                           .val.val},
        .as = _new_name.str()};
  }
};

template <class StructType, AggregationOp _agg,
          rfl::internal::StringLiteral _name>
struct MakeField<StructType, Aggregation<_agg, Col<_name>>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A column required in the aggregation does not exist.");

  static_assert(
      std::is_integral_v<
          remove_nullable_t<underlying_t<StructType, Col<_name>>>> ||
          std::is_floating_point_v<
              remove_nullable_t<underlying_t<StructType, Col<_name>>>>,
      "Values inside the aggregation must be numerical.");

  static constexpr bool is_aggregation = true;
  static constexpr bool is_column = true;

  using Name = Literal<_name>;
  using Type = rfl::field_type_t<_name, StructType>;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    using DynamicAggregationType = dynamic_aggregation_t<_agg>;
    return dynamic::SelectFrom::Field{
        dynamic::Operation{.val = dynamic::Aggregation{DynamicAggregationType{
                               .val = dynamic::Column{.name = _name.str()}}}}};
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct MakeField<StructType, Aggregation<AggregationOp::count, Col<_name>>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A column required in the COUNT or COUNT_DISTINCT aggregation "
                "does not exist.");

  static constexpr bool is_aggregation = true;
  static constexpr bool is_column = true;

  using Name = Literal<_name>;
  using Type = size_t;

  dynamic::SelectFrom::Field operator()(const auto& _agg) const {
    return dynamic::SelectFrom::Field{dynamic::Operation{
        .val = dynamic::Aggregation{dynamic::Aggregation::Count{
            .val = dynamic::Column{.name = _name.str()},
            .distinct = _agg.distinct}},
    }};
  }
};

template <class StructType>
struct MakeField<StructType, Aggregation<AggregationOp::count, All>> {
  static constexpr bool is_aggregation = true;
  static constexpr bool is_column = true;

  using Name = Nothing;
  using Type = size_t;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{dynamic::Operation{
        .val = dynamic::Aggregation{
            dynamic::Aggregation::Count{.val = std::nullopt, .distinct = false},
        }}};
  }
};

template <class StructType, Operator _op, class Operand1Type,
          class Operand2Type>
struct MakeField<StructType, Operation<_op, Operand1Type, Operand2Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;

  using Name = Nothing;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    using DynamicOperatorType = dynamic_operator_t<_op>;
    constexpr auto num_operands = num_operands_v<_op>;
    if constexpr (num_operands == 2) {
      return dynamic::SelectFrom::Field{dynamic::Operation{DynamicOperatorType{
          .op1 = MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                     _o.operand1)
                     .val,
          .op2 = MakeField<StructType, std::remove_cvref_t<Operand2Type>>{}(
                     _o.operand2)
                     .val}}};
    }
  }
};

template <class StructType, class ValueType>
inline dynamic::SelectFrom::Field make_field(const ValueType& _val) {
  return MakeField<std::remove_cvref_t<StructType>,
                   std::remove_cvref_t<ValueType>>{}(_val);
}

}  // namespace sqlgen::transpilation

#endif
