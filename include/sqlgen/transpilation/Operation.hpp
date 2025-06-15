#ifndef SQLGEN_TRANSPILATION_OPERATION_HPP_
#define SQLGEN_TRANSPILATION_OPERATION_HPP_

#include <string>
#include <type_traits>

#include "../Result.hpp"
#include "Operator.hpp"

namespace sqlgen::transpilation {

template <Operator _op, class _Operand1Type, class _Operand2Type = Nothing>
struct Operation {
  static constexpr Operator op = _op;

  using Operand1Type = _Operand1Type;
  using Operand2Type = _Operand2Type;

  Operand1Type operand1;
  Operand2Type operand2;
};

}  // namespace sqlgen::transpilation

#endif
