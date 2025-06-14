#ifndef SQLGEN_TRANSPILATION_OPERATIONS_HPP_
#define SQLGEN_TRANSPILATION_OPERATIONS_HPP_

#include <string>

namespace sqlgen::transpilation::operations {

template <class _Op1Type, class _Op2Type>
struct Divides {
  using Op1Type = _Op1Type;
  using Op2Type = _Op2Type;

  Op1Type op1;
  Op2Type op2;
};

template <class _Op1Type, class _Op2Type>
struct Minus {
  using Op1Type = _Op1Type;
  using Op2Type = _Op2Type;

  Op1Type op1;
  Op2Type op2;
};

template <class _Op1Type, class _Op2Type>
struct Multiplies {
  using Op1Type = _Op1Type;
  using Op2Type = _Op2Type;

  Op1Type op1;
  Op2Type op2;
};

template <class _Op1Type, class _Op2Type>
struct Plus {
  using Op1Type = _Op1Type;
  using Op2Type = _Op2Type;

  Op1Type op1;
  Op2Type op2;
};

}  // namespace sqlgen::transpilation::operations

#endif
