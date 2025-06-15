#ifndef SQLGEN_TRANSPILATION_AGGREGATION_HPP_
#define SQLGEN_TRANSPILATION_AGGREGATION_HPP_

#include <string>

#include "AggregationOp.hpp"
#include "As.hpp"

namespace sqlgen::transpilation {

/// To be used when we want to count everything.
struct All {};

template <AggregationOp _agg, class _ValueType>
struct Aggregation {
  static constexpr auto agg = _agg;
  using ValueType = _ValueType;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    using T = std::remove_cvref_t<decltype(*this)>;
    return transpilation::As<T, _new_name>{.val = *this};
  }

  ValueType val;
  bool distinct = false;
};

}  // namespace sqlgen::transpilation

#endif
