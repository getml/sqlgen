#ifndef SQLGEN_TRANSPILATION_OPERATION_HPP_
#define SQLGEN_TRANSPILATION_OPERATION_HPP_

#include <string>
#include <type_traits>

#include "../Result.hpp"
#include "Operator.hpp"
#include "to_transpilation_type.hpp"

namespace sqlgen::transpilation {

template <Operator _op, class _Operand1Type, class _Operand2Type = Nothing>
struct Operation {
  static constexpr Operator op = _op;

  using Operand1Type = _Operand1Type;
  using Operand2Type = _Operand2Type;

  Operand1Type operand1;
  Operand2Type operand2;

  template <class T>
  friend auto operator/(const Operation& _op1, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return Operation<Operator::divides,
                     Operation<_op, _Operand1Type, _Operand2Type>, OtherType>{
        .operand1 = _op1, .operand2 = to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator-(const Operation& _op1, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return Operation<Operator::minus,
                     Operation<_op, _Operand1Type, _Operand2Type>, OtherType>{
        .operand1 = _op1, .operand2 = to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator*(const Operation& _op1, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return Operation<Operator::multiplies,
                     Operation<_op, _Operand1Type, _Operand2Type>, OtherType>{
        .operand1 = _op1, .operand2 = to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator+(const Operation& _op1, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return Operation<Operator::plus,
                     Operation<_op, _Operand1Type, _Operand2Type>, OtherType>{
        .operand1 = _op1, .operand2 = to_transpilation_type(_op2)};
  }
};

template <Operator _op, class _Operand1Type, class _Operand2Type>
struct ToTranspilationType<Operation<_op, _Operand1Type, _Operand2Type>> {
  using Type = Operation<_op, _Operand1Type, _Operand2Type>;

  Type operator()(const Type& _val) const noexcept { return _val; }
};

}  // namespace sqlgen::transpilation

#endif
