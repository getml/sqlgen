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
    std::optional<std::string> as;
  };

  struct Count {
    std::optional<Column> val;
    bool distinct = false;
    std::optional<std::string> as;
  };

  struct Max {
    ColumnOrValue val;
    std::optional<std::string> as;
  };

  struct Min {
    ColumnOrValue val;
    std::optional<std::string> as;
  };

  struct Sum {
    ColumnOrValue val;
    std::optional<std::string> as;
  };

  using ReflectionType = rfl::TaggedUnion<"what", Avg, Count, Max, Min, Sum>;

  const ReflectionType& reflection() const { return val; }

  ReflectionType val;
};

}  // namespace sqlgen::dynamic

#endif
