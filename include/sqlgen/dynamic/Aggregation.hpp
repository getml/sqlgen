#ifndef SQLGEN_DYNAMIC_AGGREGATION_HPP_
#define SQLGEN_DYNAMIC_AGGREGATION_HPP_

#include <optional>
#include <rfl.hpp>
#include <string>
#include <vector>

#include "Column.hpp"
#include "ColumnOrValue.hpp"

namespace sqlgen::dynamic {

struct Aggregation {
  struct Avg {
    ColumnOrValue val;
  };

  struct Count {
    std::optional<Column> val;
    bool distinct = false;
  };

  struct Max {
    ColumnOrValue val;
  };

  struct Min {
    ColumnOrValue val;
  };

  struct Sum {
    ColumnOrValue val;
  };

  using ReflectionType = rfl::TaggedUnion<"what", Avg, Count, Max, Min, Sum>;

  const ReflectionType& reflection() const { return val; }

  ReflectionType val;
};

}  // namespace sqlgen::dynamic

#endif
