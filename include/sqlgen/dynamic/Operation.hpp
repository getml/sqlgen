#ifndef SQLGEN_DYNAMIC_OPERATION_HPP_
#define SQLGEN_DYNAMIC_OPERATION_HPP_

#include <rfl.hpp>
#include <vector>

#include "../Ref.hpp"
#include "Aggregation.hpp"
#include "Column.hpp"
#include "Type.hpp"
#include "Value.hpp"

namespace sqlgen::dynamic {

struct Operation {
  struct Abs {
    Ref<Operation> op1;
  };

  struct Cast {
    Ref<Operation> op1;
    Type target_type;
  };

  struct Ceil {
    Ref<Operation> op1;
  };

  struct Coalesce {
    std::vector<Ref<Operation>> ops;
  };

  struct Concat {
    std::vector<Ref<Operation>> ops;
  };

  struct Cos {
    Ref<Operation> op1;
  };

  struct Divides {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Exp {
    Ref<Operation> op1;
  };

  struct Floor {
    Ref<Operation> op1;
  };

  struct Length {
    Ref<Operation> op1;
  };

  struct Ln {
    Ref<Operation> op1;
  };

  struct Lower {
    Ref<Operation> op1;
  };

  struct LTrim {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Log2 {
    Ref<Operation> op1;
  };

  struct Minus {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Mod {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Multiplies {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Plus {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Replace {
    Ref<Operation> op1;
    Ref<Operation> op2;
    Ref<Operation> op3;
  };

  struct Round {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct RTrim {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Sin {
    Ref<Operation> op1;
  };

  struct Sqrt {
    Ref<Operation> op1;
  };

  struct Tan {
    Ref<Operation> op1;
  };

  struct Trim {
    Ref<Operation> op1;
    Ref<Operation> op2;
  };

  struct Upper {
    Ref<Operation> op1;
  };

  using ReflectionType =
      rfl::TaggedUnion<"what", Abs, Aggregation, Cast, Ceil, Column, Coalesce,
                       Concat, Cos, Divides, Exp, Floor, Length, Ln, Log2,
                       Lower, LTrim, Minus, Mod, Multiplies, Plus, Replace,
                       Round, RTrim, Sin, Sqrt, Tan, Trim, Upper, Value>;

  const ReflectionType& reflection() const { return val; }

  ReflectionType val;
};

}  // namespace sqlgen::dynamic

#endif
