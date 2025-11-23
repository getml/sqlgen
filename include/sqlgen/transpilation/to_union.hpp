#ifndef SQLGEN_TRANSPILATION_TO_UNION_HPP_
#define SQLGEN_TRANSPILATION_TO_UNION_HPP_

#include <rfl.hpp>
#include <rfl/named_tuple_t.hpp>
#include <vector>

#include "../Ref.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "../dynamic/Union.hpp"
#include "table_tuple_t.hpp"
#include "to_select_from.hpp"
#include "value_t.hpp"

namespace sqlgen::transpilation {

template <class ContainerType, class... SelectTs>
dynamic::Union to_union(const rfl::Tuple<SelectTs...>& _stmts,
                        const bool _all) noexcept {
  using ValueType = value_t<ContainerType>;
  using NamedTupleType = rfl::named_tuple_t<ValueType>;

  const auto columns = NamedTupleType::Names::names();

  const auto selects = rfl::apply(
      [](const auto... _stmt) {
        return Ref<std::vector<dynamic::SelectFrom>>::make(
            std::vector<dynamic::SelectFrom>({to_select_from<
                table_tuple_t<typename SelectTs::TableOrQueryType,
                              typename SelectTs::AliasType,
                              typename SelectTs::JoinsType>,
                typename SelectTs::AliasType, typename SelectTs::FieldsType,
                typename SelectTs::TableOrQueryType,
                typename SelectTs::JoinsType, typename SelectTs::WhereType,
                typename SelectTs::GroupByType, typename SelectTs::OrderByType,
                typename SelectTs::LimitType>(_stmt.fields_, _stmt.from_,
                                              _stmt.joins_, _stmt.where_,
                                              _stmt.limit_)...}));
      },
      _stmts);

  return dynamic::Union{.columns = columns, .selects = selects, .all = _all};
}

}  // namespace sqlgen::transpilation

#endif
