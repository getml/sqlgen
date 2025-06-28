#ifndef SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_
#define SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_

#include <rfl.hpp>
#include <string>
#include <type_traits>

#include "Aggregation.hpp"
#include "AggregationOp.hpp"
#include "Col.hpp"
#include "Desc.hpp"
#include "Operation.hpp"
#include "Value.hpp"
#include "all_columns_exist.hpp"
#include "dynamic_operator_t.hpp"
#include "is_nullable.hpp"
#include "remove_nullable_t.hpp"
#include "remove_reflection_t.hpp"

namespace sqlgen::transpilation {

template <class T, class _Type>
struct Underlying;

template <class T, AggregationOp _op, class ValueType>
struct Underlying<T, Aggregation<_op, ValueType>> {
  using Type = typename Underlying<T, std::remove_cvref_t<ValueType>>::Type;
};

template <class T, rfl::internal::StringLiteral _name>
struct Underlying<T, Col<_name>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  using Type = remove_reflection_t<rfl::field_type_t<_name, T>>;
};

template <class T, rfl::internal::StringLiteral _name>
struct Underlying<T, Desc<Col<_name>>> {
  using Type = remove_reflection_t<rfl::field_type_t<_name, T>>;
};

template <class T, class Operand1Type, class TargetType>
struct Underlying<
    T, Operation<Operator::cast, Operand1Type, TypeHolder<TargetType>>> {
  using Type =
      std::conditional_t<is_nullable_v<typename Underlying<
                             T, std::remove_cvref_t<Operand1Type>>::Type>,
                         std::optional<std::remove_cvref_t<TargetType>>,
                         std::remove_cvref_t<TargetType>>;
};

template <class T, class Head, class... Tail>
struct Underlying<T, Operation<Operator::coalesce, rfl::Tuple<Head, Tail...>>> {
  using Operand1Type = typename Underlying<T, std::remove_cvref_t<Head>>::Type;

  static_assert((true && ... &&
                 std::is_same_v<remove_nullable_t<Operand1Type>,
                                remove_nullable_t<typename Underlying<
                                    T, std::remove_cvref_t<Tail>>::Type>>),
                "All inputs into coalesce(...) must have the same type.");

  using Type = std::conditional_t<
      (is_nullable_v<Operand1Type> && ... &&
       is_nullable_v<typename Underlying<T, std::remove_cvref_t<Tail>>::Type>),
      std::optional<remove_nullable_t<Operand1Type>>,
      remove_nullable_t<Operand1Type>>;
};

template <class T, class... OperandTypes>
struct Underlying<T, Operation<Operator::concat, rfl::Tuple<OperandTypes...>>> {
  static_assert(
      (true && ... &&
       std::is_same_v<remove_nullable_t<typename Underlying<
                          T, std::remove_cvref_t<OperandTypes>>::Type>,
                      std::string>),
      "Must be a string");

  using Type =
      std::conditional_t<(false || ... ||
                          is_nullable_v<typename Underlying<
                              T, std::remove_cvref_t<OperandTypes>>::Type>),
                         std::optional<std::string>, std::string>;
};

template <class T, rfl::internal::StringLiteral _name, class... DurationTypes>
struct Underlying<T, Operation<Operator::date_plus_duration, Col<_name>,
                               rfl::Tuple<DurationTypes...>>> {
  using Type = typename Underlying<std::remove_cvref_t<T>, Col<_name>>::Type;
};

template <class T, class Operand1Type, class Operand2Type, class Operand3Type>
struct Underlying<
    T, Operation<Operator::replace, Operand1Type, Operand2Type, Operand3Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;
  using Underlying2 =
      typename Underlying<T, std::remove_cvref_t<Operand2Type>>::Type;
  using Underlying3 =
      typename Underlying<T, std::remove_cvref_t<Operand3Type>>::Type;

  static_assert(std::is_same_v<remove_nullable_t<Underlying1>, std::string>,
                "Must be a string");
  static_assert(std::is_same_v<remove_nullable_t<Underlying2>, std::string>,
                "Must be a string");
  static_assert(std::is_same_v<remove_nullable_t<Underlying3>, std::string>,
                "Must be a string");

  using Type = std::conditional_t<is_nullable_v<Underlying1> ||
                                      is_nullable_v<Underlying2> ||
                                      is_nullable_v<Underlying3>,
                                  std::optional<std::string>, std::string>;
};

template <class T, class Operand1Type, class Operand2Type>
struct Underlying<T, Operation<Operator::round, Operand1Type, Operand2Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;
  using Underlying2 =
      typename Underlying<T, std::remove_cvref_t<Operand2Type>>::Type;

  static_assert(std::is_integral_v<remove_nullable_t<Underlying1>> ||
                    std::is_floating_point_v<remove_nullable_t<Underlying1>>,
                "Must be a numerical type");
  static_assert(std::is_integral_v<Underlying2>, "Must be an integral type");

  using Type = Underlying1;
};

template <class T, Operator _op, class Operand1Type>
  requires((num_operands_v<_op>) == 1 &&
           (operator_category_v<_op>) == OperatorCategory::string)
struct Underlying<T, Operation<_op, Operand1Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;

  static_assert(std::is_same_v<remove_nullable_t<Underlying1>, std::string>,
                "Must be a string");

  using StringType =
      std::conditional_t<is_nullable_v<Underlying1>, std::optional<std::string>,
                         std::string>;
  using SizeType = std::conditional_t<is_nullable_v<Underlying1>,
                                      std::optional<size_t>, size_t>;
  using Type =
      std::conditional_t<_op == Operator::length, SizeType, StringType>;
};

template <class T, Operator _op, class Operand1Type, class Operand2Type>
  requires((num_operands_v<_op>) == 2 &&
           (operator_category_v<_op>) == OperatorCategory::string)
struct Underlying<T, Operation<_op, Operand1Type, Operand2Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;
  using Underlying2 =
      typename Underlying<T, std::remove_cvref_t<Operand2Type>>::Type;

  static_assert(std::is_same_v<remove_nullable_t<Underlying1>, std::string>,
                "Must be a string");
  static_assert(std::is_same_v<remove_nullable_t<Underlying2>, std::string>,
                "Must be a string");

  using Type = std::conditional_t<is_nullable_v<Underlying1> ||
                                      is_nullable_v<Underlying2>,
                                  std::optional<std::string>, std::string>;
};

template <class T, Operator _op, class Operand1Type>
  requires((num_operands_v<_op>) == 1 &&
           (operator_category_v<_op>) == OperatorCategory::numerical)
struct Underlying<T, Operation<_op, Operand1Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;

  static_assert(std::is_integral_v<remove_nullable_t<Underlying1>> ||
                    std::is_floating_point_v<remove_nullable_t<Underlying1>>,
                "Must be a numerical type");

  using Type = Underlying1;
};

template <class T, Operator _op, class Operand1Type, class Operand2Type>
  requires((num_operands_v<_op>) == 2 &&
           (operator_category_v<_op>) == OperatorCategory::numerical)
struct Underlying<T, Operation<_op, Operand1Type, Operand2Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;
  using Underlying2 =
      typename Underlying<T, std::remove_cvref_t<Operand2Type>>::Type;

  static_assert(
      requires(remove_nullable_t<Underlying1> op1,
               remove_nullable_t<Underlying2> op2) { op1 + op2; },
      "Binary operations are not possible on these types.");

  using ResultType = std::invoke_result_t<
      decltype([](const auto& op1, const auto& op2) { return op1 + op2; }),
      remove_nullable_t<Underlying1>, remove_nullable_t<Underlying2>>;

  using Type = std::conditional_t<is_nullable_v<Underlying1> ||
                                      is_nullable_v<Underlying2>,
                                  std::optional<ResultType>, ResultType>;
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
