#ifndef SQLGEN_DYNAMIC_CONDITION_HPP_
#define SQLGEN_DYNAMIC_CONDITION_HPP_

#include <rfl.hpp>

#include "../Ref.hpp"
#include "Column.hpp"
#include "ColumnOrValue.hpp"
#include "Operation.hpp"

namespace sqlgen::dynamic {

struct Condition {
  struct And {
    Ref<Condition> cond1;
    Ref<Condition> cond2;
  };

  struct Equal {
    Operation op1;
    Operation op2;
  };

  struct GreaterEqual {
    Operation op1;
    Operation op2;
  };

  struct GreaterThan {
    Operation op1;
    Operation op2;
  };

  struct IsNotNull {
    Operation op;
  };

  struct IsNull {
    Operation op;
  };

  struct LesserEqual {
    Operation op1;
    Operation op2;
  };

  struct LesserThan {
    Operation op1;
    Operation op2;
  };

  struct Like {
    Operation op;
    dynamic::Value pattern;
  };

  struct NotEqual {
    Operation op1;
    Operation op2;
  };

  struct NotLike {
    Operation op;
    dynamic::Value pattern;
  };

  struct Or {
    Ref<Condition> cond1;
    Ref<Condition> cond2;
  };

  using ReflectionType =
      rfl::TaggedUnion<"what", And, Equal, GreaterEqual, GreaterThan, IsNull,
                       IsNotNull, LesserEqual, LesserThan, Like, NotEqual,
                       NotLike, Or>;

  const ReflectionType& reflection() const { return val; }

  ReflectionType val;
};

}  // namespace sqlgen::dynamic

#endif
