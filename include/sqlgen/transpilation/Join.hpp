#ifndef SQLGEN_TRANSPILATION_JOIN_HPP_
#define SQLGEN_TRANSPILATION_JOIN_HPP_

#include <rfl.hpp>

#include "../Literal.hpp"
#include "../dynamic/JoinType.hpp"

namespace sqlgen::transpilation {

using JoinType = dynamic::JoinType;

template <class TableOrQueryType, class ConditionType,
          rfl::internal::StringLiteral _alias>
struct Join {
  using Alias = Literal<_alias>;

  JoinType how;
  TableOrQueryType table_or_query;
  ConditionType on;
};

}  // namespace sqlgen::transpilation

#endif
