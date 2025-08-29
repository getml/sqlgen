#ifndef SQLGEN_DYNAMIC_INSERT_HPP_
#define SQLGEN_DYNAMIC_INSERT_HPP_

#include <string>
#include <vector>

#include "Table.hpp"

namespace sqlgen::dynamic {

struct Insert {
  Table table;
  std::vector<std::string> columns;
  bool or_replace;
};

}  // namespace sqlgen::dynamic

#endif
