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
#include "to_value.hpp"
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

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToCondition<T, conditions::Equal<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>, Col<_name2>>(),
                "All columns must exist.");
  static_assert(std::equality_comparable_with<underlying_t<T, Col<_name1>>,
                                              underlying_t<T, Col<_name2>>>,
                "Must be equality comparable.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::Equal{
                                  .op1 = dynamic::Column{.name = _name1.str()},
                                  .op2 = dynamic::Column{.name = _name2.str()},
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name, class V>
struct ToCondition<T, conditions::Equal<Col<_name>, Value<V>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(std::equality_comparable_with<underlying_t<T, Col<_name>>,
                                              underlying_t<T, Value<V>>>,
                "Must be equality comparable.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::Equal{
                                  .op1 = dynamic::Column{.name = _name.str()},
                                  .op2 = to_value(_cond.op2.val),
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToCondition<T, conditions::GreaterEqual<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>, Col<_name2>>(),
                "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name1>>,
                                          underlying_t<T, Col<_name2>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::GreaterEqual{
                                  .op1 = dynamic::Column{.name = _name1.str()},
                                  .op2 = dynamic::Column{.name = _name2.str()},
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name, class V>
struct ToCondition<T, conditions::GreaterEqual<Col<_name>, Value<V>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name>>,
                                          underlying_t<T, Value<V>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::GreaterEqual{
                                  .op1 = dynamic::Column{.name = _name.str()},
                                  .op2 = to_value(_cond.op2.val),
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToCondition<T, conditions::GreaterThan<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>, Col<_name2>>(),
                "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name1>>,
                                          underlying_t<T, Col<_name2>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::GreaterThan{
                                  .op1 = dynamic::Column{.name = _name1.str()},
                                  .op2 = dynamic::Column{.name = _name2.str()},
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name, class V>
struct ToCondition<T, conditions::GreaterThan<Col<_name>, Value<V>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name>>,
                                          underlying_t<T, Value<V>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::GreaterThan{
                                  .op1 = dynamic::Column{.name = _name.str()},
                                  .op2 = to_value(_cond.op2.val),
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToCondition<T, conditions::LesserEqual<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>, Col<_name2>>(),
                "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name1>>,
                                          underlying_t<T, Col<_name2>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::LesserEqual{
                                  .op1 = dynamic::Column{.name = _name1.str()},
                                  .op2 = dynamic::Column{.name = _name2.str()},
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name, class V>
struct ToCondition<T, conditions::LesserEqual<Col<_name>, Value<V>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name>>,
                                          underlying_t<T, Value<V>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::LesserEqual{
                                  .op1 = dynamic::Column{.name = _name.str()},
                                  .op2 = to_value(_cond.op2.val),
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToCondition<T, conditions::LesserThan<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>, Col<_name2>>(),
                "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name1>>,
                                          underlying_t<T, Col<_name2>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::LesserThan{
                                  .op1 = dynamic::Column{.name = _name1.str()},
                                  .op2 = dynamic::Column{.name = _name2.str()},
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name, class V>
struct ToCondition<T, conditions::LesserThan<Col<_name>, Value<V>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(std::totally_ordered_with<underlying_t<T, Col<_name>>,
                                          underlying_t<T, Value<V>>>,
                "Must be totally ordered.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::LesserThan{
                                  .op1 = dynamic::Column{.name = _name.str()},
                                  .op2 = to_value(_cond.op2.val),
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name>
struct ToCondition<T, conditions::Like<Col<_name>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(
      std::equality_comparable_with<underlying_t<T, Col<_name>>,
                                    underlying_t<T, Value<std::string>>>,
      "Must be equality comparable with a string.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::Like{
                                  .op = dynamic::Column{.name = _name.str()},
                                  .pattern = to_value(_cond.pattern)}};
  }
};

template <class T, rfl::internal::StringLiteral _name>
struct ToCondition<T, conditions::IsNotNull<Col<_name>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");

  dynamic::Condition operator()(const auto&) const {
    return dynamic::Condition{.val = dynamic::Condition::IsNotNull{
                                  .op = dynamic::Column{.name = _name.str()}}};
  }
};

template <class T, rfl::internal::StringLiteral _name>
struct ToCondition<T, conditions::IsNull<Col<_name>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");

  dynamic::Condition operator()(const auto&) const {
    return dynamic::Condition{.val = dynamic::Condition::IsNull{
                                  .op = dynamic::Column{.name = _name.str()}}};
  }
};

template <class T, rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
struct ToCondition<T, conditions::NotEqual<Col<_name1>, Col<_name2>>> {
  static_assert(all_columns_exist<T, Col<_name1>, Col<_name2>>(),
                "All columns must exist.");
  static_assert(std::equality_comparable_with<underlying_t<T, Col<_name1>>,
                                              underlying_t<T, Col<_name2>>>,
                "Must be equality comparable.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::NotEqual{
                                  .op1 = dynamic::Column{.name = _name1.str()},
                                  .op2 = dynamic::Column{.name = _name2.str()},
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name, class V>
struct ToCondition<T, conditions::NotEqual<Col<_name>, Value<V>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(std::equality_comparable_with<underlying_t<T, Col<_name>>,
                                              underlying_t<T, Value<V>>>,
                "Must be equality comparable.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::NotEqual{
                                  .op1 = dynamic::Column{.name = _name.str()},
                                  .op2 = to_value(_cond.op2.val),
                              }};
  }
};

template <class T, rfl::internal::StringLiteral _name>
struct ToCondition<T, conditions::NotLike<Col<_name>>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  static_assert(
      std::equality_comparable_with<underlying_t<T, Col<_name>>,
                                    underlying_t<T, Value<std::string>>>,
      "Must be equality comparable with a string.");

  dynamic::Condition operator()(const auto& _cond) const {
    return dynamic::Condition{.val = dynamic::Condition::NotLike{
                                  .op = dynamic::Column{.name = _name.str()},
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
