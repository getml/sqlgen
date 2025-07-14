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

template <rfl::internal::StringLiteral _name>
auto operator"" _t1() {
  return Col<_name, "t1">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t2() {
  return Col<_name, "t2">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t3() {
  return Col<_name, "t3">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t4() {
  return Col<_name, "t4">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t5() {
  return Col<_name, "t5">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t6() {
  return Col<_name, "t6">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t7() {
  return Col<_name, "t7">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t8() {
  return Col<_name, "t8">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t9() {
  return Col<_name, "t9">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t10() {
  return Col<_name, "t10">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t11() {
  return Col<_name, "t11">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t12() {
  return Col<_name, "t12">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t13() {
  return Col<_name, "t13">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t14() {
  return Col<_name, "t14">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t15() {
  return Col<_name, "t15">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t16() {
  return Col<_name, "t16">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t17() {
  return Col<_name, "t17">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t18() {
  return Col<_name, "t18">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t19() {
  return Col<_name, "t19">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t20() {
  return Col<_name, "t20">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t21() {
  return Col<_name, "t21">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t22() {
  return Col<_name, "t22">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t23() {
  return Col<_name, "t23">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t24() {
  return Col<_name, "t24">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t25() {
  return Col<_name, "t25">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t26() {
  return Col<_name, "t26">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t27() {
  return Col<_name, "t27">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t28() {
  return Col<_name, "t28">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t29() {
  return Col<_name, "t29">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t30() {
  return Col<_name, "t30">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t31() {
  return Col<_name, "t31">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t32() {
  return Col<_name, "t32">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t33() {
  return Col<_name, "t33">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t34() {
  return Col<_name, "t34">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t35() {
  return Col<_name, "t35">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t36() {
  return Col<_name, "t36">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t37() {
  return Col<_name, "t37">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t38() {
  return Col<_name, "t38">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t39() {
  return Col<_name, "t39">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t40() {
  return Col<_name, "t40">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t41() {
  return Col<_name, "t41">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t42() {
  return Col<_name, "t42">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t43() {
  return Col<_name, "t43">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t44() {
  return Col<_name, "t44">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t45() {
  return Col<_name, "t45">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t46() {
  return Col<_name, "t46">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t47() {
  return Col<_name, "t47">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t48() {
  return Col<_name, "t48">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t49() {
  return Col<_name, "t49">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t50() {
  return Col<_name, "t50">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t51() {
  return Col<_name, "t51">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t52() {
  return Col<_name, "t52">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t53() {
  return Col<_name, "t53">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t54() {
  return Col<_name, "t54">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t55() {
  return Col<_name, "t55">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t56() {
  return Col<_name, "t56">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t57() {
  return Col<_name, "t57">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t58() {
  return Col<_name, "t58">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t59() {
  return Col<_name, "t59">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t60() {
  return Col<_name, "t60">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t61() {
  return Col<_name, "t61">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t62() {
  return Col<_name, "t62">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t63() {
  return Col<_name, "t63">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t64() {
  return Col<_name, "t64">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t65() {
  return Col<_name, "t65">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t66() {
  return Col<_name, "t66">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t67() {
  return Col<_name, "t67">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t68() {
  return Col<_name, "t68">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t69() {
  return Col<_name, "t69">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t70() {
  return Col<_name, "t70">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t71() {
  return Col<_name, "t71">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t72() {
  return Col<_name, "t72">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t73() {
  return Col<_name, "t73">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t74() {
  return Col<_name, "t74">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t75() {
  return Col<_name, "t75">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t76() {
  return Col<_name, "t76">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t77() {
  return Col<_name, "t77">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t78() {
  return Col<_name, "t78">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t79() {
  return Col<_name, "t79">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t80() {
  return Col<_name, "t80">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t81() {
  return Col<_name, "t81">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t82() {
  return Col<_name, "t82">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t83() {
  return Col<_name, "t83">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t84() {
  return Col<_name, "t84">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t85() {
  return Col<_name, "t85">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t86() {
  return Col<_name, "t86">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t87() {
  return Col<_name, "t87">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t88() {
  return Col<_name, "t88">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t89() {
  return Col<_name, "t89">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t90() {
  return Col<_name, "t90">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t91() {
  return Col<_name, "t91">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t92() {
  return Col<_name, "t92">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t93() {
  return Col<_name, "t93">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t94() {
  return Col<_name, "t94">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t95() {
  return Col<_name, "t95">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t96() {
  return Col<_name, "t96">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t97() {
  return Col<_name, "t97">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t98() {
  return Col<_name, "t98">{};
}

template <rfl::internal::StringLiteral _name>
auto operator"" _t99() {
  return Col<_name, "t99">{};
}

}  // namespace sqlgen

#endif
