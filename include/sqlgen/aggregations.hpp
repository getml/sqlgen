#ifndef SQLGEN_AGGREGATIONS_HPP_
#define SQLGEN_AGGREGATIONS_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "col.hpp"
#include "transpilation/aggregations.hpp"

namespace sqlgen {

template <rfl::internal::StringLiteral _name>
auto avg(const Col<_name>&) {
  return transpilation::aggregations::Avg<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

inline auto count() {
  return transpilation::aggregations::Count<transpilation::aggregations::All>{};
}

template <rfl::internal::StringLiteral _name>
auto count(const Col<_name>&) {
  return transpilation::aggregations::Count<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <rfl::internal::StringLiteral _name>
auto count_distinct(const Col<_name>&) {
  return transpilation::aggregations::Count<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}, .distinct = true};
}

template <rfl::internal::StringLiteral _name>
auto max(const Col<_name>&) {
  return transpilation::aggregations::Max<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <rfl::internal::StringLiteral _name>
auto min(const Col<_name>&) {
  return transpilation::aggregations::Min<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <rfl::internal::StringLiteral _name>
auto sum(const Col<_name>&) {
  return transpilation::aggregations::Sum<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

}  // namespace sqlgen

#endif
