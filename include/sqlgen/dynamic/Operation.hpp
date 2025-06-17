#ifndef SQLGEN_DYNAMIC_OPERATION_HPP_
#define SQLGEN_DYNAMIC_OPERATION_HPP_

#include <rfl.hpp>

#include "../Ref.hpp"
#include "Aggregation.hpp"
#include "Column.hpp"
#include "ColumnOrValue.hpp"
#include "Value.hpp"

namespace sqlgen::dynamic {

struct Operation {
  struct Divides {
    Ref<Operation> op1;
    Ref<Operation> op2;
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

  using ReflectionType = rfl::TaggedUnion<"what", Aggregation, Column, Divides,
                                          Minus, Mod, Multiplies, Plus, Value>;

  const ReflectionType& reflection() const { return val; }

  ReflectionType val;
};

}  // namespace sqlgen::dynamic

#endif
