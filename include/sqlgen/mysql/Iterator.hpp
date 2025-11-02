#ifndef SQLGEN_MYSQL_ITERATOR_HPP_
#define SQLGEN_MYSQL_ITERATOR_HPP_

#include <mysql.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../IteratorBase.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../sqlgen_api.hpp"
#include "Connection.hpp"

namespace sqlgen::mysql {

class SQLGEN_API Iterator : public sqlgen::IteratorBase {
  using ConnPtr = Ref<MYSQL>;
  using ResPtr = Ref<MYSQL_RES>;

 public:
  Iterator(const ResPtr& _res, const ConnPtr& _conn);

  ~Iterator();

  /// Whether the end of the available data has been reached.
  bool end() const final { return end_; }

  /// Returns the next batch of rows.
  /// If _batch_size is greater than the number of rows left, returns all
  /// of the rows left.
  Result<std::vector<std::vector<std::optional<std::string>>>> next(
      const size_t _batch_size) final;

 private:
  /// The underlying mysql result.
  ResPtr res_;

  /// The underlying mysql connection. We have this in here to prevent its
  /// destruction for the lifetime of the iterator.
  ConnPtr conn_;

  /// Whether the end is reached.
  bool end_;
};

}  // namespace sqlgen::mysql

#endif
