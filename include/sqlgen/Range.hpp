#ifndef SQLGEN_RANGE_HPP_
#define SQLGEN_RANGE_HPP_

#include <memory>
#include <ranges>

#include "Iterator.hpp"
#include "Result.hpp"

namespace sqlgen {

/// This class is meant to provide a way to iterate through the data in the
/// database efficiently that is compatible with std::ranges.
template <class IteratorType>
  requires std::input_iterator<IteratorType>
class Range {
 public:
  using value_type = typename IteratorType::value_type;

  struct End {};

  Range(const IteratorType& _it) : it_(_it) {}

  ~Range() = default;

  auto begin() const { return it_; }

  auto end() const { return typename IteratorType::End{}; }

 private:
  /// The underlying database iterator.
  IteratorType it_;
};

}  // namespace sqlgen

#endif
