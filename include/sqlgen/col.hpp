#ifndef SQLGEN_COL_HPP_
#define SQLGEN_COL_HPP_

#include <chrono>
#include <rfl.hpp>
#include <string>

#include "transpilation/As.hpp"
#include "transpilation/Col.hpp"
#include "transpilation/Condition.hpp"
#include "transpilation/Desc.hpp"
#include "transpilation/Operation.hpp"
#include "transpilation/Operator.hpp"
#include "transpilation/Set.hpp"
#include "transpilation/Value.hpp"
#include "transpilation/conditions.hpp"
#include "transpilation/is_duration.hpp"
#include "transpilation/to_transpilation_type.hpp"

namespace sqlgen {

template <rfl::internal::StringLiteral _name>
struct Col {
  using ColType = transpilation::Col<_name>;
  using Name = rfl::Literal<_name>;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    return transpilation::As<transpilation::Col<_name>, _new_name>{
        .val = transpilation::Col<_name>{}};
  }

  /// Signals to order_by that this column is to be sorted in descending order.
  auto desc() const noexcept {
    return transpilation::Desc<transpilation::Col<_name>>{};
  }

  /// Returns the column name.
  std::string name() const noexcept { return Name().str(); }

  /// Returns an IS NULL condition.
  auto is_null() const noexcept {
    return transpilation::make_condition(
        transpilation::conditions::is_null(transpilation::Col<_name>{}));
  }

  /// Returns a IS NOT NULL condition.
  auto is_not_null() const noexcept {
    return transpilation::make_condition(
        transpilation::conditions::is_not_null(transpilation::Col<_name>{}));
  }

  /// Returns a LIKE condition.
  auto like(const std::string& _pattern) const noexcept {
    return transpilation::make_condition(
        transpilation::conditions::like(transpilation::Col<_name>{}, _pattern));
  }

  /// Returns a NOT LIKE condition.
  auto not_like(const std::string& _pattern) const noexcept {
    return transpilation::make_condition(transpilation::conditions::not_like(
        transpilation::Col<_name>{}, _pattern));
  }

  /// Returns a SET clause in an UPDATE statement.
  template <class T>
  auto set(const T& _to) const noexcept {
    return transpilation::Set<transpilation::Col<_name>,
                              std::remove_cvref_t<T>>{.to = _to};
  }

  /// Returns a SET clause in an UPDATE statement.
  template <rfl::internal::StringLiteral _other_name>
  auto set(const Col<_other_name>& _to) const noexcept {
    return transpilation::Set<transpilation::Col<_name>,
                              transpilation::Col<_other_name>>{
        .to = transpilation::Col<_other_name>{}};
  }

  /// Returns a SET clause in an UPDATE statement.
  auto set(const char* _to) const noexcept {
    return transpilation::Set<transpilation::Col<_name>, std::string>{.to =
                                                                          _to};
  }

  template <class T>
  friend auto operator==(const Col&, const T& _t) {
    return transpilation::make_condition(transpilation::conditions::equal(
        transpilation::Col<_name>{}, transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator!=(const Col&, const T& _t) {
    return transpilation::make_condition(transpilation::conditions::not_equal(
        transpilation::Col<_name>{}, transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator<(const Col&, const T& _t) {
    return transpilation::make_condition(transpilation::conditions::lesser_than(
        transpilation::Col<_name>{}, transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator<=(const Col&, const T& _t) {
    return transpilation::make_condition(
        transpilation::conditions::lesser_equal(
            transpilation::Col<_name>{},
            transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator>(const Col&, const T& _t) {
    return transpilation::make_condition(
        transpilation::conditions::greater_than(
            transpilation::Col<_name>{},
            transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator>=(const Col&, const T& _t) {
    return transpilation::make_condition(
        transpilation::conditions::greater_equal(
            transpilation::Col<_name>{},
            transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator/(const Col&, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return transpilation::Operation<transpilation::Operator::divides,
                                    transpilation::Col<_name>, OtherType>{
        .operand1 = transpilation::Col<_name>{},
        .operand2 = transpilation::to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator-(const Col& _op1, const T& _op2) noexcept {
    if constexpr (transpilation::is_duration_v<T>) {
      using DurationType = std::remove_cvref_t<T>;
      return _op1 + DurationType(_op2.count() * (-1));

    } else {
      using OtherType = typename transpilation::ToTranspilationType<
          std::remove_cvref_t<T>>::Type;

      return transpilation::Operation<transpilation::Operator::minus,
                                      transpilation::Col<_name>, OtherType>{
          .operand1 = transpilation::Col<_name>{},
          .operand2 = transpilation::to_transpilation_type(_op2)};
    }
  }

  template <class T>
  friend auto operator%(const Col&, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return transpilation::Operation<transpilation::Operator::mod,
                                    transpilation::Col<_name>, OtherType>{
        .operand1 = transpilation::Col<_name>{},
        .operand2 = transpilation::to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator*(const Col&, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return transpilation::Operation<transpilation::Operator::multiplies,
                                    transpilation::Col<_name>, OtherType>{
        .operand1 = transpilation::Col<_name>{},
        .operand2 = transpilation::to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator+(const Col&, const T& _op2) noexcept {
    if constexpr (transpilation::is_duration_v<T>) {
      using DurationType = std::remove_cvref_t<T>;
      return transpilation::Operation<
          transpilation::Operator::date_plus_duration,
          transpilation::Col<_name>, rfl::Tuple<DurationType>>{
          .operand1 = transpilation::Col<_name>{},
          .operand2 = rfl::Tuple<DurationType>(_op2)};

    } else {
      using OtherType = typename transpilation::ToTranspilationType<
          std::remove_cvref_t<T>>::Type;

      return transpilation::Operation<transpilation::Operator::plus,
                                      transpilation::Col<_name>, OtherType>{
          .operand1 = transpilation::Col<_name>{},
          .operand2 = transpilation::to_transpilation_type(_op2)};
    }
  }
};

template <rfl::internal::StringLiteral _name>
const auto col = Col<_name>{};

template <rfl::internal::StringLiteral _name>
auto operator"" _c() {
  return Col<_name>{};
}

namespace transpilation {

template <rfl::internal::StringLiteral _name>
struct ToTranspilationType<sqlgen::Col<_name>> {
  using Type = transpilation::Col<_name>;

  Type operator()(const auto&) const noexcept {
    return transpilation::Col<_name>{};
  }
};

}  // namespace transpilation

}  // namespace sqlgen

#endif
