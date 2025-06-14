#ifndef SQLGEN_DYNAMIC_SELECTFROM_HPP_
#define SQLGEN_DYNAMIC_SELECTFROM_HPP_

#include <optional>
#include <rfl.hpp>
#include <string>
#include <vector>

#include "Condition.hpp"
#include "GroupBy.hpp"
#include "Limit.hpp"
#include "Operation.hpp"
#include "OrderBy.hpp"
#include "Table.hpp"

namespace sqlgen::dynamic {

struct SelectFrom {
  struct Field {
    Operation val;
    std::optional<std::string> as;
  };

  Table table;
  std::vector<Field> fields;
  std::optional<Condition> where = std::nullopt;
  std::optional<GroupBy> group_by = std::nullopt;
  std::optional<OrderBy> order_by = std::nullopt;
  std::optional<Limit> limit = std::nullopt;
};

}  // namespace sqlgen::dynamic

#endif
