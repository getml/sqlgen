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

template <rfl::internal::StringLiteral _name,
          rfl::internal::StringLiteral _alias = "">
struct Col {
  using ColType = transpilation::Col<_name, _alias>;
  using Name = rfl::Literal<_name>;
  using Alias = rfl::Literal<_alias>;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    return transpilation::As<transpilation::Col<_name, _alias>, _new_name>{
        .val = transpilation::Col<_name, _alias>{}};
  }

  /// Signals to order_by that this column is to be sorted in descending order.
  auto desc() const noexcept {
    return transpilation::Desc<transpilation::Col<_name, _alias>>{};
  }

  /// Returns the column name.
  std::string name() const noexcept { return Name().str(); }

  /// Returns an IS NULL condition.
  auto is_null() const noexcept {
    return transpilation::make_condition(transpilation::conditions::is_null(
        transpilation::Col<_name, _alias>{}));
  }

  /// Returns a IS NOT NULL condition.
  auto is_not_null() const noexcept {
    return transpilation::make_condition(transpilation::conditions::is_not_null(
        transpilation::Col<_name, _alias>{}));
  }

  /// Returns a LIKE condition.
  auto like(const std::string& _pattern) const noexcept {
    return transpilation::make_condition(transpilation::conditions::like(
        transpilation::Col<_name, _alias>{}, _pattern));
  }

  /// Returns a NOT LIKE condition.
  auto not_like(const std::string& _pattern) const noexcept {
    return transpilation::make_condition(transpilation::conditions::not_like(
        transpilation::Col<_name, _alias>{}, _pattern));
  }

  /// Returns a SET clause in an UPDATE statement.
  template <class T>
  auto set(const T& _to) const noexcept {
    return transpilation::Set<transpilation::Col<_name, _alias>,
                              std::remove_cvref_t<T>>{.to = _to};
  }

  /// Returns a SET clause in an UPDATE statement.
  template <rfl::internal::StringLiteral _other_name,
            rfl::internal::StringLiteral _other_alias>
  auto set(const Col<_other_name, _other_alias>& _to) const noexcept {
    return transpilation::Set<transpilation::Col<_name, _alias>,
                              transpilation::Col<_other_name, _other_alias>>{
        .to = transpilation::Col<_other_name, _other_alias>{}};
  }

  /// Returns a SET clause in an UPDATE statement.
  auto set(const char* _to) const noexcept {
    return transpilation::Set<transpilation::Col<_name, _alias>, std::string>{
        .to = _to};
  }

  template <class T>
  friend auto operator==(const Col&, const T& _t) {
    return transpilation::make_condition(transpilation::conditions::equal(
        transpilation::Col<_name, _alias>{},
        transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator!=(const Col&, const T& _t) {
    return transpilation::make_condition(transpilation::conditions::not_equal(
        transpilation::Col<_name, _alias>{},
        transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator<(const Col&, const T& _t) {
    return transpilation::make_condition(transpilation::conditions::lesser_than(
        transpilation::Col<_name, _alias>{},
        transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator<=(const Col&, const T& _t) {
    return transpilation::make_condition(
        transpilation::conditions::lesser_equal(
            transpilation::Col<_name, _alias>{},
            transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator>(const Col&, const T& _t) {
    return transpilation::make_condition(
        transpilation::conditions::greater_than(
            transpilation::Col<_name, _alias>{},
            transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator>=(const Col&, const T& _t) {
    return transpilation::make_condition(
        transpilation::conditions::greater_equal(
            transpilation::Col<_name, _alias>{},
            transpilation::to_transpilation_type(_t)));
  }

  template <class T>
  friend auto operator/(const Col&, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return transpilation::Operation<transpilation::Operator::divides,
                                    transpilation::Col<_name, _alias>,
                                    OtherType>{
        .operand1 = transpilation::Col<_name, _alias>{},
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
                                      transpilation::Col<_name, _alias>,
                                      OtherType>{
          .operand1 = transpilation::Col<_name, _alias>{},
          .operand2 = transpilation::to_transpilation_type(_op2)};
    }
  }

  template <class T>
  friend auto operator%(const Col&, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return transpilation::Operation<transpilation::Operator::mod,
                                    transpilation::Col<_name, _alias>,
                                    OtherType>{
        .operand1 = transpilation::Col<_name, _alias>{},
        .operand2 = transpilation::to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator*(const Col&, const T& _op2) noexcept {
    using OtherType = typename transpilation::ToTranspilationType<
        std::remove_cvref_t<T>>::Type;

    return transpilation::Operation<transpilation::Operator::multiplies,
                                    transpilation::Col<_name, _alias>,
                                    OtherType>{
        .operand1 = transpilation::Col<_name, _alias>{},
        .operand2 = transpilation::to_transpilation_type(_op2)};
  }

  template <class T>
  friend auto operator+(const Col&, const T& _op2) noexcept {
    if constexpr (transpilation::is_duration_v<T>) {
      using DurationType = std::remove_cvref_t<T>;
      return transpilation::Operation<
          transpilation::Operator::date_plus_duration,
          transpilation::Col<_name, _alias>, rfl::Tuple<DurationType>>{
          .operand1 = transpilation::Col<_name, _alias>{},
          .operand2 = rfl::Tuple<DurationType>(_op2)};

    } else {
      using OtherType = typename transpilation::ToTranspilationType<
          std::remove_cvref_t<T>>::Type;

      return transpilation::Operation<transpilation::Operator::plus,
                                      transpilation::Col<_name, _alias>,
                                      OtherType>{
          .operand1 = transpilation::Col<_name, _alias>{},
          .operand2 = transpilation::to_transpilation_type(_op2)};
    }
  }
};

template <rfl::internal::StringLiteral _name,
          rfl::internal::StringLiteral _alias = "">
const auto col = Col<_name, _alias>{};

namespace transpilation {

template <rfl::internal::StringLiteral _name,
          rfl::internal::StringLiteral _alias>
struct ToTranspilationType<sqlgen::Col<_name, _alias>> {
  using Type = transpilation::Col<_name, _alias>;

  Type operator()(const auto&) const noexcept {
    return transpilation::Col<_name, _alias>{};
  }
};
}  // namespace transpilation

template <rfl::internal::StringLiteral _name>
auto operator"" _c() {
  return Col<_name>{};
}

#define SQLGEN_TABLE_ALIAS_LITERAL(N)           \
  template <rfl::internal::StringLiteral _name> \
  auto operator""_t##N() {                      \
    return Col<_name, "t" #N>{};                \
  }

SQLGEN_TABLE_ALIAS_LITERAL(1)
SQLGEN_TABLE_ALIAS_LITERAL(2)
SQLGEN_TABLE_ALIAS_LITERAL(3)
SQLGEN_TABLE_ALIAS_LITERAL(4)
SQLGEN_TABLE_ALIAS_LITERAL(5)
SQLGEN_TABLE_ALIAS_LITERAL(6)
SQLGEN_TABLE_ALIAS_LITERAL(7)
SQLGEN_TABLE_ALIAS_LITERAL(8)
SQLGEN_TABLE_ALIAS_LITERAL(9)
SQLGEN_TABLE_ALIAS_LITERAL(10)
SQLGEN_TABLE_ALIAS_LITERAL(11)
SQLGEN_TABLE_ALIAS_LITERAL(12)
SQLGEN_TABLE_ALIAS_LITERAL(13)
SQLGEN_TABLE_ALIAS_LITERAL(14)
SQLGEN_TABLE_ALIAS_LITERAL(15)
SQLGEN_TABLE_ALIAS_LITERAL(16)
SQLGEN_TABLE_ALIAS_LITERAL(17)
SQLGEN_TABLE_ALIAS_LITERAL(18)
SQLGEN_TABLE_ALIAS_LITERAL(19)
SQLGEN_TABLE_ALIAS_LITERAL(20)
SQLGEN_TABLE_ALIAS_LITERAL(21)
SQLGEN_TABLE_ALIAS_LITERAL(22)
SQLGEN_TABLE_ALIAS_LITERAL(23)
SQLGEN_TABLE_ALIAS_LITERAL(24)
SQLGEN_TABLE_ALIAS_LITERAL(25)
SQLGEN_TABLE_ALIAS_LITERAL(26)
SQLGEN_TABLE_ALIAS_LITERAL(27)
SQLGEN_TABLE_ALIAS_LITERAL(28)
SQLGEN_TABLE_ALIAS_LITERAL(29)
SQLGEN_TABLE_ALIAS_LITERAL(30)
SQLGEN_TABLE_ALIAS_LITERAL(31)
SQLGEN_TABLE_ALIAS_LITERAL(32)
SQLGEN_TABLE_ALIAS_LITERAL(33)
SQLGEN_TABLE_ALIAS_LITERAL(34)
SQLGEN_TABLE_ALIAS_LITERAL(35)
SQLGEN_TABLE_ALIAS_LITERAL(36)
SQLGEN_TABLE_ALIAS_LITERAL(37)
SQLGEN_TABLE_ALIAS_LITERAL(38)
SQLGEN_TABLE_ALIAS_LITERAL(39)
SQLGEN_TABLE_ALIAS_LITERAL(40)
SQLGEN_TABLE_ALIAS_LITERAL(41)
SQLGEN_TABLE_ALIAS_LITERAL(42)
SQLGEN_TABLE_ALIAS_LITERAL(43)
SQLGEN_TABLE_ALIAS_LITERAL(44)
SQLGEN_TABLE_ALIAS_LITERAL(45)
SQLGEN_TABLE_ALIAS_LITERAL(46)
SQLGEN_TABLE_ALIAS_LITERAL(47)
SQLGEN_TABLE_ALIAS_LITERAL(48)
SQLGEN_TABLE_ALIAS_LITERAL(49)
SQLGEN_TABLE_ALIAS_LITERAL(50)
SQLGEN_TABLE_ALIAS_LITERAL(51)
SQLGEN_TABLE_ALIAS_LITERAL(52)
SQLGEN_TABLE_ALIAS_LITERAL(53)
SQLGEN_TABLE_ALIAS_LITERAL(54)
SQLGEN_TABLE_ALIAS_LITERAL(55)
SQLGEN_TABLE_ALIAS_LITERAL(56)
SQLGEN_TABLE_ALIAS_LITERAL(57)
SQLGEN_TABLE_ALIAS_LITERAL(58)
SQLGEN_TABLE_ALIAS_LITERAL(59)
SQLGEN_TABLE_ALIAS_LITERAL(60)
SQLGEN_TABLE_ALIAS_LITERAL(61)
SQLGEN_TABLE_ALIAS_LITERAL(62)
SQLGEN_TABLE_ALIAS_LITERAL(63)
SQLGEN_TABLE_ALIAS_LITERAL(64)
SQLGEN_TABLE_ALIAS_LITERAL(65)
SQLGEN_TABLE_ALIAS_LITERAL(66)
SQLGEN_TABLE_ALIAS_LITERAL(67)
SQLGEN_TABLE_ALIAS_LITERAL(68)
SQLGEN_TABLE_ALIAS_LITERAL(69)
SQLGEN_TABLE_ALIAS_LITERAL(70)
SQLGEN_TABLE_ALIAS_LITERAL(71)
SQLGEN_TABLE_ALIAS_LITERAL(72)
SQLGEN_TABLE_ALIAS_LITERAL(73)
SQLGEN_TABLE_ALIAS_LITERAL(74)
SQLGEN_TABLE_ALIAS_LITERAL(75)
SQLGEN_TABLE_ALIAS_LITERAL(76)
SQLGEN_TABLE_ALIAS_LITERAL(77)
SQLGEN_TABLE_ALIAS_LITERAL(78)
SQLGEN_TABLE_ALIAS_LITERAL(79)
SQLGEN_TABLE_ALIAS_LITERAL(80)
SQLGEN_TABLE_ALIAS_LITERAL(81)
SQLGEN_TABLE_ALIAS_LITERAL(82)
SQLGEN_TABLE_ALIAS_LITERAL(83)
SQLGEN_TABLE_ALIAS_LITERAL(84)
SQLGEN_TABLE_ALIAS_LITERAL(85)
SQLGEN_TABLE_ALIAS_LITERAL(86)
SQLGEN_TABLE_ALIAS_LITERAL(87)
SQLGEN_TABLE_ALIAS_LITERAL(88)
SQLGEN_TABLE_ALIAS_LITERAL(89)
SQLGEN_TABLE_ALIAS_LITERAL(90)
SQLGEN_TABLE_ALIAS_LITERAL(91)
SQLGEN_TABLE_ALIAS_LITERAL(92)
SQLGEN_TABLE_ALIAS_LITERAL(93)
SQLGEN_TABLE_ALIAS_LITERAL(94)
SQLGEN_TABLE_ALIAS_LITERAL(95)
SQLGEN_TABLE_ALIAS_LITERAL(96)
SQLGEN_TABLE_ALIAS_LITERAL(97)
SQLGEN_TABLE_ALIAS_LITERAL(98)
SQLGEN_TABLE_ALIAS_LITERAL(99)

}  // namespace sqlgen

#endif
