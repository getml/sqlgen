#ifndef SQLGEN_DYNAMIC_INSERT_HPP_
#define SQLGEN_DYNAMIC_INSERT_HPP_

#include <string>
#include <vector>

#include "Table.hpp"

namespace sqlgen::dynamic {

struct Insert {
  enum class ConflictPolicy { none, replace, ignore };

  Table table;
  std::vector<std::string> columns;
  ConflictPolicy conflict_policy = ConflictPolicy::none;
  std::vector<std::string> non_primary_keys;

  /// Holds primary keys and unique columns when conflict_policy is replace.
  std::vector<std::string> constraints;

  /// The columns to be returned after insert.
  std::vector<std::string> returning;
};

}  // namespace sqlgen::dynamic

#endif
