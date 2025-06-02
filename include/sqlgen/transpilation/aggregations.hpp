#ifndef SQLGEN_TRANSPILATION_AGGREGATIONS_HPP_
#define SQLGEN_TRANSPILATION_AGGREGATIONS_HPP_

#include <string>

namespace sqlgen::transpilation::aggregations {

template <class _ValueType>
struct Avg {
  using ValueType = _ValueType;

  ValueType val;
};

template <class _ValueType>
struct Count {
  using ValueType = _ValueType;

  ValueType val;
  bool distinct = false;
};

template <class _ValueType>
struct Max {
  using ValueType = _ValueType;

  ValueType val;
};

template <class _ValueType>
struct Min {
  using ValueType = _ValueType;

  ValueType val;
};

template <class _ValueType>
struct Sum {
  using ValueType = _ValueType;

  ValueType val;
};

}  // namespace sqlgen::transpilation::aggregations

#endif
