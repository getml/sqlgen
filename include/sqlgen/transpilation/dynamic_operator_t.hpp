#ifndef SQLGEN_TRANSPILATION_DYNAMICOPERATORT_HPP_
#define SQLGEN_TRANSPILATION_DYNAMICOPERATORT_HPP_

#include "../dynamic/Operation.hpp"
#include "Operator.hpp"
#include "OperatorCategory.hpp"

namespace sqlgen::transpilation {

template <Operator op>
struct DynamicOperator;

template <>
struct DynamicOperator<Operator::abs> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Abs;
};

template <>
struct DynamicOperator<Operator::ceil> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Ceil;
};

template <>
struct DynamicOperator<Operator::cos> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Cos;
};

template <>
struct DynamicOperator<Operator::divides> {
  static constexpr size_t num_operands = 2;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Divides;
};

template <>
struct DynamicOperator<Operator::exp> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Exp;
};

template <>
struct DynamicOperator<Operator::floor> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Floor;
};

template <>
struct DynamicOperator<Operator::ln> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Ln;
};

template <>
struct DynamicOperator<Operator::log2> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Log2;
};

template <>
struct DynamicOperator<Operator::minus> {
  static constexpr size_t num_operands = 2;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Minus;
};

template <>
struct DynamicOperator<Operator::mod> {
  static constexpr size_t num_operands = 2;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Mod;
};

template <>
struct DynamicOperator<Operator::multiplies> {
  static constexpr size_t num_operands = 2;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Multiplies;
};

template <>
struct DynamicOperator<Operator::plus> {
  static constexpr size_t num_operands = 2;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Plus;
};

template <>
struct DynamicOperator<Operator::sin> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Sin;
};

template <>
struct DynamicOperator<Operator::sqrt> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Sqrt;
};

template <>
struct DynamicOperator<Operator::tan> {
  static constexpr size_t num_operands = 1;
  static constexpr auto category = OperatorCategory::numerical;
  using Type = dynamic::Operation::Tan;
};

template <Operator op>
using dynamic_operator_t = typename DynamicOperator<op>::Type;

template <Operator op>
inline constexpr size_t num_operands_v = DynamicOperator<op>::num_operands;

template <Operator op>
inline constexpr auto operator_category_v = DynamicOperator<op>::category;

}  // namespace sqlgen::transpilation

#endif
