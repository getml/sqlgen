#ifndef SQLGEN_AGGREGATIONS_HPP_
#define SQLGEN_AGGREGATIONS_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "col.hpp"
#include "transpilation/aggregations.hpp"

namespace sqlgen {

template <rfl::internal::StringLiteral _name>
auto avg(const Col<_name>& _col) {
  return transpilation::aggregations::Avg<transpilation::Col<_name>>{
      .val = transpilation::Col<_name>{}};
}

template <class ValueType>
auto count(const ValueType& _val) {
  return transpilation::aggregations::Count<std::remove_cvref_t<ValueType>>{
      .val = _val};
}

template <class ValueType>
auto count_distinct(const ValueType& _val) {
  return transpilation::aggregations::Count<std::remove_cvref_t<ValueType>>{
      .val = _val, .distinct = true};
}

template <class ValueType>
auto max(const ValueType& _val) {
  return transpilation::aggregations::Max<std::remove_cvref_t<ValueType>>{
      .val = _val};
}

template <class ValueType>
auto min(const ValueType& _val) {
  return transpilation::aggregations::Min<std::remove_cvref_t<ValueType>>{
      .val = _val};
}

template <class ValueType>
auto sum(const ValueType& _val) {
  return transpilation::aggregations::Sum<std::remove_cvref_t<ValueType>>{
      .val = _val};
}

}  // namespace sqlgen

#endif
