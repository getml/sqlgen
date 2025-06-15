#ifndef SQLGEN_TRANSPILATION_DYNAMICOPERATORT_HPP_
#define SQLGEN_TRANSPILATION_DYNAMICOPERATORT_HPP_

#include "../dynamic/Operation.hpp"
#include "Operator.hpp"

namespace sqlgen::transpilation {

template <Operator op>
struct DynamicOperator;

template <>
struct DynamicOperator<Operator::divides> {
  static constexpr size_t num_operands = 2;
  using Type = dynamic::Operation::Divides;
};

template <>
struct DynamicOperator<Operator::minus> {
  static constexpr size_t num_operands = 2;
  using Type = dynamic::Operation::Minus;
};

template <>
struct DynamicOperator<Operator::multiplies> {
  static constexpr size_t num_operands = 2;
  using Type = dynamic::Operation::Multiplies;
};

template <>
struct DynamicOperator<Operator::plus> {
  static constexpr size_t num_operands = 2;
  using Type = dynamic::Operation::Plus;
};

template <Operator op>
using dynamic_operator_t = typename DynamicOperator<op>::Type;

template <Operator op>
inline constexpr size_t num_operands_v = DynamicOperator<op>::num_operands;

}  // namespace sqlgen::transpilation

#endif
