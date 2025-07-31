#ifndef SQLGEN_DYNAMIC_CREATETABLEAS_HPP_
#define SQLGEN_DYNAMIC_CREATETABLEAS_HPP_

#include "SelectFrom.hpp"
#include "Table.hpp"

namespace sqlgen::dynamic {

struct CreateTableAs {
  Table table;
  SelectFrom as;
  bool if_not_exists;
};

}  // namespace sqlgen::dynamic

#endif
