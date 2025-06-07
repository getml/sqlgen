#ifndef SQLGEN_COL_HPP_
#define SQLGEN_COL_HPP_

#include <rfl.hpp>
#include <string>

#include "transpilation/As.hpp"
#include "transpilation/Col.hpp"
#include "transpilation/Condition.hpp"
#include "transpilation/Desc.hpp"
#include "transpilation/Set.hpp"
#include "transpilation/Value.hpp"
#include "transpilation/conditions.hpp"

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
  return transpilation::make_condition(transpilation::conditions::equal(
      transpilation::Col<_name1>{}, transpilation::Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator==(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::equal(
      transpilation::Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator!=(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(transpilation::conditions::not_equal(
      transpilation::Col<_name1>{}, transpilation::Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator!=(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::not_equal(
      transpilation::Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator<(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(transpilation::conditions::lesser_than(
      transpilation::Col<_name1>{}, transpilation::Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator<(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::lesser_than(
      transpilation::Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator<=(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(transpilation::conditions::lesser_equal(
      transpilation::Col<_name1>{}, transpilation::Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator<=(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::lesser_equal(
      transpilation::Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator>(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(transpilation::conditions::greater_than(
      transpilation::Col<_name1>{}, transpilation::Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator>(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::greater_than(
      transpilation::Col<_name1>{}, transpilation::make_value(_t)));
}

template <rfl::internal::StringLiteral _name1,
          rfl::internal::StringLiteral _name2>
auto operator>=(const Col<_name1>&, const Col<_name2>&) {
  return transpilation::make_condition(transpilation::conditions::greater_equal(
      transpilation::Col<_name1>{}, transpilation::Col<_name2>{}));
}

template <rfl::internal::StringLiteral _name1, class T>
auto operator>=(const Col<_name1>&, const T& _t) {
  return transpilation::make_condition(transpilation::conditions::greater_equal(
      transpilation::Col<_name1>{}, transpilation::make_value(_t)));
}

}  // namespace sqlgen

#endif
