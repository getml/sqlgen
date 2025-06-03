#ifndef SQLGEN_TRANSPILATION_TO_AGGREGATION_HPP_
#define SQLGEN_TRANSPILATION_TO_AGGREGATION_HPP_

#include <rfl.hpp>

#include "../Literal.hpp"
#include "As.hpp"
#include "Col.hpp"
#include "aggregations.hpp"
#include "all_columns_exist.hpp"
#include "to_value.hpp"

namespace sqlgen::transpilation {

template <class StructType, class Agg>
struct ToAggregation;

template <class StructType, class _ValueType,
          class rfl::internal::StringLiteral _new_name>
struct ToAggregation<As<ValueType, _new_name>> {
  dynamic::Aggregation operator()(const auto& _as) const {
    return ToAggregation<StructType, _ValueType>(_as.val).val.visit([](auto v) {
      return dynamic::Aggregation{.val = v, .as = _new_name.str()};
    });
  }
};

template <class StructType, rfl::internal::StringLiteral _name>
struct Aggregation<aggregations::Avg<Col<_name>>> {
  static_assert(all_columns_exist<StructType, Col<_name>>(),
                "A column required in Avg aggregation does not exist.");

  dynamic::Aggregation operator()(const auto& _agg) const {
    return dynamic::Aggregation{
        dynamic::Aggregation::Avg{.val = dynamic::Column{.name = _name.str()}}};
  }
};

template <class StructType, class ValueType>
struct Aggregation<aggregations::Avg<ValueType>> {
  dynamic::Aggregation operator()(const auto& _agg) const {
    return dynamic::Aggregation{
        dynamic::Aggregation::Avg{.val = to_value(_agg.val)}};
  }
};

template <class T, class ConditionType>
std::optional<dynamic::Condition> to_condition(const ConditionType& _cond) {
  return ToCondition<T, std::remove_cvref_t<ConditionType>>{}(_cond);
}

}  // namespace sqlgen::transpilation

#endif
