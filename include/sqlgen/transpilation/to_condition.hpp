#ifndef SQLGEN_TRANSPILATION_TO_CONDITION_HPP_
#define SQLGEN_TRANSPILATION_TO_CONDITION_HPP_

#include <concepts>
#include <optional>
#include <type_traits>
#include <vector>

#include "../Ref.hpp"
#include "../Result.hpp"
#include "../dynamic/Condition.hpp"
#include "Condition.hpp"
#include "all_columns_exist.hpp"
#include "conditions.hpp"
#include "make_field.hpp"
#include "to_transpilation_type.hpp"
#include "underlying_t.hpp"

namespace sqlgen::transpilation {

template <class T, class ConditionType>
struct ToCondition;

template <class T, class CondType>
struct ToCondition<T, Condition<CondType>> {
  dynamic::Condition operator()(const Condition<CondType>& _cond) const {
    return ToCondition<T, std::remove_cvref_t<CondType>>{}(_cond.condition);
  }
};

template <class T, class CondType1, class CondType2>
struct ToCondition<T, conditions::And<CondType1, CondType2>> {
  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::And{
            .cond1 = Ref<dynamic::Condition>::make(
                ToCondition<T, std::remove_cvref_t<CondType1>>{}(_cond.cond1)),
            .cond2 = Ref<dynamic::Condition>::make(
                ToCondition<T, std::remove_cvref_t<CondType2>>{}(_cond.cond2)),
        }};
  }
};

template <class T, class Op1Type, class Op2Type>
struct ToCondition<T, conditions::Equal<Op1Type, Op2Type>> {
  static_assert(std::equality_comparable_with<underlying_t<T, Op1Type>,
                                              underlying_t<T, Op2Type>>,
                "Must be equality comparable.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::Equal{.op1 = make_field<T>(_cond.op1).val,
                                         .op2 = make_field<T>(_cond.op2).val}};
  }
};

template <class T, class Op1Type, class Op2Type>
struct ToCondition<T, conditions::GreaterEqual<Op1Type, Op2Type>> {
  static_assert(std::totally_ordered_with<underlying_t<T, Op1Type>,
                                          underlying_t<T, Op2Type>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::GreaterEqual{
                                  .op1 = make_field<T>(_cond.op1).val,
                                  .op2 = make_field<T>(_cond.op2).val}};
  }
};

template <class T, class Op1Type, class Op2Type>
struct ToCondition<T, conditions::GreaterThan<Op1Type, Op2Type>> {
  static_assert(std::totally_ordered_with<underlying_t<T, Op1Type>,
                                          underlying_t<T, Op2Type>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::GreaterThan{
                                  .op1 = make_field<T>(_cond.op1).val,
                                  .op2 = make_field<T>(_cond.op2).val}};
  }
};

template <class T, class Op1Type, class Op2Type>
struct ToCondition<T, conditions::LesserEqual<Op1Type, Op2Type>> {
  static_assert(std::totally_ordered_with<underlying_t<T, Op1Type>,
                                          underlying_t<T, Op2Type>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::LesserEqual{
                                  .op1 = make_field<T>(_cond.op1).val,
                                  .op2 = make_field<T>(_cond.op2).val}};
  }
};

template <class T, class Op1Type, class Op2Type>
struct ToCondition<T, conditions::LesserThan<Op1Type, Op2Type>> {
  static_assert(std::totally_ordered_with<underlying_t<T, Op1Type>,
                                          underlying_t<T, Op2Type>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::LesserThan{
                                  .op1 = make_field<T>(_cond.op1).val,
                                  .op2 = make_field<T>(_cond.op2).val}};
  }
};

template <class T, class OpType>
struct ToCondition<T, conditions::Like<OpType>> {
  static_assert(
      std::equality_comparable_with<underlying_t<T, OpType>,
                                    underlying_t<T, Value<std::string>>>,
      "Must be equality comparable with a string.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::Like{.op = make_field<T>(_cond.op).val,
                                        .pattern = to_value(_cond.pattern)}};
  }
};

template <class T, class OpType>
struct ToCondition<T, conditions::IsNotNull<OpType>> {
  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::IsNotNull{
                                  .op = make_field<T>(_cond.op).val}};
  }
};

template <class T, class OpType>
struct ToCondition<T, conditions::IsNull<OpType>> {
  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::IsNull{.op = make_field<T>(_cond.op).val}};
  }
};

template <class T, class CondType>
struct ToCondition<T, conditions::Not<CondType>> {
  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::Not{
            .cond = Ref<dynamic::Condition>::make(
                ToCondition<T, std::remove_cvref_t<CondType>>{}(_cond.cond))}};
  }
};

template <class T, class Op1Type, class Op2Type>
struct ToCondition<T, conditions::NotEqual<Op1Type, Op2Type>> {
  static_assert(std::equality_comparable_with<underlying_t<T, Op1Type>,
                                              underlying_t<T, Op2Type>>,
                "Must be equality comparable.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::NotEqual{
                                  .op1 = make_field<T>(_cond.op1).val,
                                  .op2 = make_field<T>(_cond.op2).val}};
  }
};

template <class T, class OpType>
struct ToCondition<T, conditions::NotLike<OpType>> {
  static_assert(
      std::equality_comparable_with<underlying_t<T, OpType>,
                                    underlying_t<T, Value<std::string>>>,
      "Must be equality comparable with a string.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::NotLike{.op = make_field<T>(_cond.op).val,
                                           .pattern = to_value(_cond.pattern)}};
  }
};

template <class T, class CondType1, class CondType2>
struct ToCondition<T, conditions::Or<CondType1, CondType2>> {
  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{
        .val = dynamic::Condition::Or{
            .cond1 = Ref<dynamic::Condition>::make(
                ToCondition<T, std::remove_cvref_t<CondType1>>{}(_cond.cond1)),
            .cond2 = Ref<dynamic::Condition>::make(
                ToCondition<T, std::remove_cvref_t<CondType2>>{}(_cond.cond2)),
        }};
  }
};

template <class T>
struct ToCondition<T, Nothing> {
  std::optional<dynamic::Condition> operator()(const Nothing&) const {
    return std::nullopt;
  }
};

template <class T, class ConditionType>
std::optional<dynamic::Condition> to_condition(const ConditionType& _cond) {
  return ToCondition<T, std::remove_cvref_t<ConditionType>>{}(_cond);
}

}  // namespace sqlgen::transpilation

#endif
