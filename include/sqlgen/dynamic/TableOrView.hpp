#ifndef SQLGEN_DYNAMIC_TABLE_OR_VIEW_HPP_
#define SQLGEN_DYNAMIC_TABLE_OR_VIEW_HPP_

namespace sqlgen::dynamic {

enum class TableOrView { table, view, materialized_view };

}  // namespace sqlgen::dynamic

#endif
