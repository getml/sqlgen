#ifndef SQLGEN_COL_HPP_
#define SQLGEN_COL_HPP_

#include <rfl.hpp>
#include <string>

#include "transpilation/Condition.hpp"
#include "transpilation/Desc.hpp"
#include "transpilation/Set.hpp"
#include "transpilation/Value.hpp"
#include "transpilation/conditions.hpp"

namespace sqlgen {

template <rfl::internal::StringLiteral _name>
struct Col {
  using Name = rfl::Literal<_name>;

  /// Signals to order_by that this column is to be sorted in descending order.
  auto desc() const noexcept { return transpilation::Desc<Col<_name>>{}; }

  /// Returns the column name.
  std::string name() const noexcept { return Name().str(); }

  /// Defines a SET clause in an UPDATE statement.
  template <class T>
  auto set(const T& _to) const noexcept {
    return transpilation::Set<Col<_name>, std::remove_cvref_t<T>>{.to = _to};
  }

  /// Defines a SET clause in an UPDATE statement.
  auto set(const char* _to) const noexcept {
    return transpilation::Set<Col<_name>, std::string>{.to = _to};
  }
};

template <rfl::internal::StringLiteral _name>
const auto col = Col<_name>{};

template <rfl::internal::StringLiteral _name>
auto operator"" _c() {
  return Col<_name>{};
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator==(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(
      transpilation::conditions::equal(Col<_name1>{}, Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator==(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::equal(
      Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator!=(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(
      transpilation::conditions::not_equal(Col<_name1>{}, Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator!=(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::not_equal(
      Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator<(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(
      transpilation::conditions::lesser_than(Col<_name1>{}, Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator<(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::lesser_than(
      Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator<=(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(
      transpilation::conditions::lesser_equal(Col<_name1>{}, Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator<=(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::lesser_equal(
      Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator>(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(
      transpilation::conditions::greater_than(Col<_name1>{}, Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator>(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::greater_than(
      Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator>=(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(
      transpilation::conditions::greater_equal(Col<_name1>{}, Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator>=(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::greater_equal(
      Col<_name1>{}, transpilation::make_value(_t)));
}

}  // namespace sqlgen

#endif
