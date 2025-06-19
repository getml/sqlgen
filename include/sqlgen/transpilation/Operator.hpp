#ifndef SQLGEN_TRANSPILATION_OPERATOR_HPP_
#define SQLGEN_TRANSPILATION_OPERATOR_HPP_

namespace sqlgen::transpilation {

enum class Operator {
  abs,
  cast,
  ceil,
  concat,
  cos,
  divides,
  exp,
  floor,
  length,
  ln,
  log2,
  lower,
  ltrim,
  minus,
  mod,
  multiplies,
  plus,
  replace,
  round,
  rtrim,
  sin,
  sqrt,
  tan,
  trim,
  upper
};

}  // namespace sqlgen::transpilation

#endif
