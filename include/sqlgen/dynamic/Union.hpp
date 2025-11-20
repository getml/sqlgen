#ifndef SQLGEN_DYNAMIC_UNION_HPP_
#define SQLGEN_DYNAMIC_UNION_HPP_

#include <vector>
#include "SelectFrom.hpp"

namespace sqlgen::dynamic {

struct Union {
  std::vector<Ref<SelectFrom>> selects_;
};

}  // namespace sqlgen::dynamic

#endif
