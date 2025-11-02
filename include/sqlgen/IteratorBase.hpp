#ifndef SQLGEN_ITERATORBASE_HPP_
#define SQLGEN_ITERATORBASE_HPP_

#include <optional>
#include <string>
#include <vector>

#include "Result.hpp"
#include "sqlgen_api.hpp"

namespace sqlgen {

/// Abstract base class for an iterator to be returned by Connection::read(...).
struct SQLGEN_API IteratorBase {
  IteratorBase();

  virtual ~IteratorBase();

  /// Whether the end of the available data has been reached.
  virtual bool end() const = 0;

  /// Returns the next batch of rows.
  /// If _batch_size is greater than the number of rows left, returns all
  /// of the rows left.
  virtual Result<std::vector<std::vector<std::optional<std::string>>>> next(
      const size_t _batch_size) = 0;
};

}  // namespace sqlgen

#endif

