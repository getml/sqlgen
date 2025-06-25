#ifndef SQLGEN_DYNAMIC_VALUE_HPP_
#define SQLGEN_DYNAMIC_VALUE_HPP_

#include <rfl.hpp>
#include <string>

#include "TimeUnit.hpp"

namespace sqlgen::dynamic {

struct Duration {
  TimeUnit unit;
  int64_t val;
};

struct Float {
  double val;
};

struct Integer {
  int64_t val;
};

struct String {
  std::string val;
};

struct Value {
  using ReflectionType =
      rfl::TaggedUnion<"type", Duration, Float, Integer, String>;
  const auto& reflection() const { return val; }
  ReflectionType val;
};

}  // namespace sqlgen::dynamic

#endif
