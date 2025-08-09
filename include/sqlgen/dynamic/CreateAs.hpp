#ifndef SQLGEN_DYNAMIC_CREATEAS_HPP_
#define SQLGEN_DYNAMIC_CREATEAS_HPP_

#include "SelectFrom.hpp"

namespace sqlgen::dynamic {

struct CreateAs {
  enum class What { table, view, materialized_view };

  What what;
  SelectFrom query;
  bool if_not_exists = true;
};

}  // namespace sqlgen::dynamic

#endif
