#ifndef SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_
#define SQLGEN_TRANSPILATION_MAKE_FIELD_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "../Literal.hpp"
#include "../Result.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "../parsing/Parser.hpp"
#include "Aggregation.hpp"
#include "AggregationOp.hpp"
#include "As.hpp"
#include "Col.hpp"
#include "Operation.hpp"
#include "Operator.hpp"
#include "OperatorCategory.hpp"
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

template <class StructType, class T>
struct MakeField {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = false;

  using Name = Nothing;
  using Type = std::remove_cvref_t<T>;

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
  static constexpr bool is_operation = false;

  using Name = Literal<_name>;
  using Type = rfl::field_type_t<_name, StructType>;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{.val = dynamic::Column{.name = _name.str()}}};
  }
};

template <class StructType, class T>
struct MakeField<StructType, Value<T>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = false;

  using Name = Nothing;
  using Type = std::remove_cvref_t<T>;

  dynamic::SelectFrom::Field operator()(const auto& _val) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{.val = to_value(_val.val)}};
  }
};

template <class StructType, class ValueType,
          rfl::internal::StringLiteral _new_name>
struct MakeField<StructType, As<ValueType, _new_name>> {
  static constexpr bool is_aggregation =
      MakeField<StructType, ValueType>::is_aggregation;
  static constexpr bool is_column = MakeField<StructType, ValueType>::is_column;
  static constexpr bool is_operation =
      MakeField<StructType, ValueType>::is_operation;

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
  static constexpr bool is_operation = false;

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
  static constexpr bool is_operation = false;

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
  static constexpr bool is_operation = false;

  using Name = Nothing;
  using Type = size_t;

  dynamic::SelectFrom::Field operator()(const auto&) const {
    return dynamic::SelectFrom::Field{dynamic::Operation{
        .val = dynamic::Aggregation{
            dynamic::Aggregation::Count{.val = std::nullopt, .distinct = false},
        }}};
  }
};

template <class StructType, class Operand1Type, class TargetType>
struct MakeField<StructType, Operation<Operator::cast, Operand1Type,
                                       TypeHolder<TargetType>>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type = std::remove_cvref_t<TargetType>;
  using Operands = rfl::Tuple<Operand1Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{dynamic::Operation::Cast{
            .op1 = Ref<dynamic::Operation>::make(
                MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                    _o.operand1)
                    .val),
            .target_type =
                parsing::Parser<std::remove_cvref_t<TargetType>>::to_type()}}};
  }
};

template <class StructType, Operator _op, class... OperandTypes>
  requires((_op == Operator::coalesce) || (_op == Operator::concat))
struct MakeField<StructType, Operation<_op, rfl::Tuple<OperandTypes...>>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type =
      underlying_t<StructType, Operation<_op, rfl::Tuple<OperandTypes...>>>;
  using Operands = rfl::Tuple<OperandTypes...>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    using DynamicOperatorType = dynamic_operator_t<_op>;
    return dynamic::SelectFrom::Field{dynamic::Operation{DynamicOperatorType{
        .ops = rfl::apply(
            [](const auto&... _ops) {
              return std::vector<Ref<dynamic::Operation>>(
                  {Ref<dynamic::Operation>::make(
                      MakeField<StructType,
                                std::remove_cvref_t<OperandTypes>>{}(_ops)
                          .val)...});
            },
            _o.operand1)}}};
  }
};

template <class StructType, class Operand1Type, class Operand2Type,
          class Operand3Type>
struct MakeField<StructType, Operation<Operator::replace, Operand1Type,
                                       Operand2Type, Operand3Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type =
      underlying_t<StructType, Operation<Operator::replace, Operand1Type,
                                         Operand2Type, Operand3Type>>;
  using Operands = rfl::Tuple<Operand1Type, Operand2Type, Operand3Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{dynamic::Operation::Replace{
            .op1 = Ref<dynamic::Operation>::make(
                MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                    _o.operand1)
                    .val),
            .op2 = Ref<dynamic::Operation>::make(
                MakeField<StructType, std::remove_cvref_t<Operand2Type>>{}(
                    _o.operand2)
                    .val),
            .op3 = Ref<dynamic::Operation>::make(
                MakeField<StructType, std::remove_cvref_t<Operand3Type>>{}(
                    _o.operand3)
                    .val)}}};
  }
};

template <class StructType, class Operand1Type, class Operand2Type>
struct MakeField<StructType,
                 Operation<Operator::round, Operand1Type, Operand2Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type =
      underlying_t<StructType,
                   Operation<Operator::round, Operand1Type, Operand2Type>>;
  using Operands = rfl::Tuple<Operand1Type, Operand2Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    return dynamic::SelectFrom::Field{
        dynamic::Operation{dynamic::Operation::Round{
            .op1 = Ref<dynamic::Operation>::make(
                MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                    _o.operand1)
                    .val),
            .op2 = Ref<dynamic::Operation>::make(
                MakeField<StructType, std::remove_cvref_t<Operand2Type>>{}(
                    _o.operand2)
                    .val)}}};
  }
};

template <class StructType, Operator _op, class Operand1Type>
  requires((num_operands_v<_op>) == 1 &&
           (operator_category_v<_op>) == OperatorCategory::string)
struct MakeField<StructType, Operation<_op, Operand1Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type = underlying_t<StructType, Operation<_op, Operand1Type>>;
  using Operands = rfl::Tuple<Operand1Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    using DynamicOperatorType = dynamic_operator_t<_op>;
    return dynamic::SelectFrom::Field{dynamic::Operation{DynamicOperatorType{
        .op1 = Ref<dynamic::Operation>::make(
            MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                _o.operand1)
                .val)}}};
  }
};

template <class StructType, Operator _op, class Operand1Type,
          class Operand2Type>
  requires((num_operands_v<_op>) == 2 &&
           (operator_category_v<_op>) == OperatorCategory::string)
struct MakeField<StructType, Operation<_op, Operand1Type, Operand2Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type =
      underlying_t<StructType, Operation<_op, Operand1Type, Operand2Type>>;
  using Operands = rfl::Tuple<Operand1Type, Operand2Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    using DynamicOperatorType = dynamic_operator_t<_op>;
    return dynamic::SelectFrom::Field{dynamic::Operation{DynamicOperatorType{
        .op1 = Ref<dynamic::Operation>::make(
            MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                _o.operand1)
                .val),
        .op2 = Ref<dynamic::Operation>::make(
            MakeField<StructType, std::remove_cvref_t<Operand2Type>>{}(
                _o.operand2)
                .val)}}};
  }
};

template <class StructType, Operator _op, class Operand1Type>
  requires((num_operands_v<_op>) == 1 &&
           (operator_category_v<_op>) == OperatorCategory::numerical)
struct MakeField<StructType, Operation<_op, Operand1Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type = underlying_t<StructType, Operation<_op, Operand1Type>>;
  using Operands = rfl::Tuple<Operand1Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    using DynamicOperatorType = dynamic_operator_t<_op>;
    return dynamic::SelectFrom::Field{dynamic::Operation{DynamicOperatorType{
        .op1 = Ref<dynamic::Operation>::make(
            MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                _o.operand1)
                .val)}}};
  }
};

template <class StructType, Operator _op, class Operand1Type,
          class Operand2Type>
  requires((num_operands_v<_op>) == 2 &&
           (operator_category_v<_op>) == OperatorCategory::numerical)
struct MakeField<StructType, Operation<_op, Operand1Type, Operand2Type>> {
  static constexpr bool is_aggregation = false;
  static constexpr bool is_column = false;
  static constexpr bool is_operation = true;

  using Name = Nothing;
  using Type =
      underlying_t<StructType, Operation<_op, Operand1Type, Operand2Type>>;
  using Operands = rfl::Tuple<Operand1Type, Operand2Type>;

  dynamic::SelectFrom::Field operator()(const auto& _o) const {
    using DynamicOperatorType = dynamic_operator_t<_op>;
    return dynamic::SelectFrom::Field{dynamic::Operation{DynamicOperatorType{
        .op1 = Ref<dynamic::Operation>::make(
            MakeField<StructType, std::remove_cvref_t<Operand1Type>>{}(
                _o.operand1)
                .val),
        .op2 = Ref<dynamic::Operation>::make(
            MakeField<StructType, std::remove_cvref_t<Operand2Type>>{}(
                _o.operand2)
                .val)}}};
  }
};

template <class StructType, class ValueType>
inline dynamic::SelectFrom::Field make_field(const ValueType& _val) {
  return MakeField<std::remove_cvref_t<StructType>,
                   std::remove_cvref_t<ValueType>>{}(_val);
}

}  // namespace sqlgen::transpilation

#endif
