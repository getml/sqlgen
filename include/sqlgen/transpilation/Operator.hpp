#ifndef SQLGEN_TRANSPILATION_OPERATOR_HPP_
#define SQLGEN_TRANSPILATION_OPERATOR_HPP_

namespace sqlgen::transpilation {

enum class Operator {
  abs,
  cast,
  ceil,
  coalesce,
  concat,
  cos,
  date_plus_duration,
  days_between,
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
  unixepoch,
  upper
};

}  // namespace sqlgen::transpilation

#endif
