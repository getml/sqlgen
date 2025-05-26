#ifndef SQLGEN_DYNAMIC_CONDITION_HPP_
#define SQLGEN_DYNAMIC_CONDITION_HPP_

#include <rfl.hpp>

#include "../Ref.hpp"
#include "Column.hpp"
#include "ColumnOrValue.hpp"

namespace sqlgen::dynamic {

struct Condition {
  struct And {
    Ref<Condition> cond1;
    Ref<Condition> cond2;
  };

  struct Equal {
    Column op1;
    ColumnOrValue op2;
  };

  struct GreaterEqual {
    Column op1;
    ColumnOrValue op2;
  };

  struct GreaterThan {
    Column op1;
    ColumnOrValue op2;
  };

  struct IsNotNull {
    Column op;
  };

  struct IsNull {
    Column op;
  };

  struct LesserEqual {
    Column op1;
    ColumnOrValue op2;
  };

  struct LesserThan {
    Column op1;
    ColumnOrValue op2;
  };

  struct Like {
    Column op;
    dynamic::Value pattern;
  };

  struct NotEqual {
    Column op1;
    ColumnOrValue op2;
  };

  struct NotLike {
    Column op;
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
