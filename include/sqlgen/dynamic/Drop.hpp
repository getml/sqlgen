#ifndef SQLGEN_DYNAMIC_DROP_HPP_
#define SQLGEN_DYNAMIC_DROP_HPP_

#include <optional>

#include "Table.hpp"

namespace sqlgen::dynamic {

struct Drop {
  enum struct What { table, view, materialized_view };
  What what;
  bool if_exists = false;
  bool cascade = false;
  Table table;
};

}  // namespace sqlgen::dynamic

#endif
