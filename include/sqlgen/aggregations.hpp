#ifndef SQLGEN_AGGREGATIONS_HPP_
#define SQLGEN_AGGREGATIONS_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "col.hpp"
#include "transpilation/Aggregation.hpp"
#include "transpilation/AggregationOp.hpp"

namespace sqlgen {

template <rfl::internal::StringLiteral _name>
auto avg(const Col<_name>&) {
  return transpilation::Aggregation<transpilation::AggregationOp::avg,
                                    transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

inline auto count() {
  return transpilation::Aggregation<transpilation::AggregationOp::count,
                                    transpilation::All>{};
}

template <rfl::internal::StringLiteral _name>
auto count(const Col<_name>&) {
  return transpilation::Aggregation<transpilation::AggregationOp::count,
                                    transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <rfl::internal::StringLiteral _name>
auto count_distinct(const Col<_name>&) {
  return transpilation::Aggregation<transpilation::AggregationOp::count,
                                    transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}, .distinct = true};
}

template <rfl::internal::StringLiteral _name>
auto max(const Col<_name>&) {
  return transpilation::Aggregation<transpilation::AggregationOp::max,
                                    transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <rfl::internal::StringLiteral _name>
auto min(const Col<_name>&) {
  return transpilation::Aggregation<transpilation::AggregationOp::min,
                                    transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <rfl::internal::StringLiteral _name>
auto sum(const Col<_name>&) {
  return transpilation::Aggregation<transpilation::AggregationOp::sum,
                                    transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

}  // namespace sqlgen

#endif
