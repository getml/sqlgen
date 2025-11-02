#ifndef SQLGEN_POSTGRES_ITERATOR_HPP_
#define SQLGEN_POSTGRES_ITERATOR_HPP_

#include <libpq-fe.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../sqlgen_api.hpp"
#include "Connection.hpp"

namespace sqlgen::postgres {

class SQLGEN_API Iterator : public sqlgen::IteratorBase {
  using ConnPtr = Ref<PGconn>;

 public:
  Iterator(const std::string& _sql, const ConnPtr& _conn);

  Iterator(const Iterator& _other) = delete;

  Iterator(Iterator&& _other) noexcept;

  ~Iterator();

  /// Whether the end of the available data has been reached.
  bool end() const final;

  /// Returns the next batch of rows.
  /// If _batch_size is greater than the number of rows left, returns all
  /// of the rows left.
  Result<std::vector<std::vector<std::optional<std::string>>>> next(
      const size_t _batch_size) final;

  Iterator& operator=(const Iterator& _other) = delete;

  Iterator& operator=(Iterator&& _other) noexcept;

 private:
  static std::string make_cursor_name() {
    // TODO: Create unique cursor names.
    return "sqlgen_cursor";
  }

  /// Shuts the iterator down.
  void shutdown();

 private:
  /// A unique name to identify the cursor.
  std::string cursor_name_;

  /// The underlying postgres connection. We have this in here to prevent its
  /// destruction for the lifetime of the iterator.
  ConnPtr conn_;

  /// Whether the end is reached.
  bool end_;
};

}  // namespace sqlgen::postgres

#endif
