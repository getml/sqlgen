#ifndef SQLGEN_TRANSPILATION_AGGREGATIONS_HPP_
#define SQLGEN_TRANSPILATION_AGGREGATIONS_HPP_

#include <string>

#include "As.hpp"

namespace sqlgen::transpilation::aggregations {

/// To be used when we want to count everything.
struct All {};

template <class _ValueType>
struct Avg {
  using ValueType = _ValueType;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    using T = std::remove_cvref_t<decltype(*this)>;
    return transpilation::As<T, _new_name>{.val = *this};
  }

  ValueType val;
};

template <class _ValueType>
struct Count {
  using ValueType = _ValueType;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    using T = std::remove_cvref_t<decltype(*this)>;
    return transpilation::As<T, _new_name>{.val = *this};
  }

  ValueType val;
  bool distinct = false;
};

template <class _ValueType>
struct Max {
  using ValueType = _ValueType;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    using T = std::remove_cvref_t<decltype(*this)>;
    return transpilation::As<T, _new_name>{.val = *this};
  }

  ValueType val;
};

template <class _ValueType>
struct Min {
  using ValueType = _ValueType;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    using T = std::remove_cvref_t<decltype(*this)>;
    return transpilation::As<T, _new_name>{.val = *this};
  }

  ValueType val;
};

template <class _ValueType>
struct Sum {
  using ValueType = _ValueType;

  template <rfl::internal::StringLiteral _new_name>
  auto as() const noexcept {
    using T = std::remove_cvref_t<decltype(*this)>;
    return transpilation::As<T, _new_name>{.val = *this};
  }

  ValueType val;
};

}  // namespace sqlgen::transpilation::aggregations

#endif
