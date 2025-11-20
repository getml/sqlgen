#ifndef SQLGEN_DYNAMIC_UNION_HPP_
#define SQLGEN_DYNAMIC_UNION_HPP_

#include <string>
#include <vector>

#include "SelectFrom.hpp"

namespace sqlgen::dynamic {

struct Union {
  std::vector<std::string> columns;
  Ref<std::vector<SelectFrom>> selects;
};

}  // namespace sqlgen::dynamic

#endif
