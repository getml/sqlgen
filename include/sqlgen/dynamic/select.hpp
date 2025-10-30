#ifndef SQLGEN_DYNAMIC_SELECT_HPP_
#define SQLGEN_DYNAMIC_SELECT_HPP_

#include <optional>
#include <string>
#include <vector>

#include "Column.hpp"
#include "SelectFrom.hpp"
#include "Table.hpp"

namespace sqlgen::dynamic {

SelectFrom select(const std::string& _table,
                  const std::vector<std::string>& _columns) {
  std::vector<Column> columns;
  for (const auto& name : _columns) {
    columns.emplace_back(Column{.name = name});
  }
  return SelectFrom{
      .table =
          Table{.alias = std::nullopt, .name = _table, .schema = std::nullopt},
      .columns = columns};
}

SelectFrom select(const std::string& _schema, const std::string& _table,
                  const std::vector<std::string>& _columns) {
  std::vector<Column> columns;
  for (const auto& name : _columns) {
    columns.emplace_back(Column{.name = name});
  }
  return SelectFrom{
      .table = Table{.alias = std::nullopt, .name = _table, .schema = _schema},
      .columns = columns};
}
}  // namespace sqlgen::dynamic
