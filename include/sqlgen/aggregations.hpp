#ifndef SQLGEN_AGGREGATIONS_HPP_
#define SQLGEN_AGGREGATIONS_HPP_

#include <type_traits>

#include "sqlgen/transpilation/aggregations.hpp"

namespace sqlgen {

template <class ValueType>
auto avg(const ValueType& _val) {
  return transpilation::aggregations::Avg<std::remove_cvref_t<ValueType>>{
      .val = _val};
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
