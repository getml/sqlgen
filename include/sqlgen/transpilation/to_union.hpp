#ifndef SQLGEN_TRANSPILATION_TO_UNION_HPP_
#define SQLGEN_TRANSPILATION_TO_UNION_HPP_

#include <variant>
#include <vector>

#include "../Ref.hpp"
#include "../dynamic/SelectFrom.hpp"
#include "../dynamic/Union.hpp"
#include "../union.hpp"
#include "to_table_or_query.hpp"

namespace sqlgen::transpilation {

template <class... Selects>
dynamic::Union to_union(const Union<Selects...>& _union) {
    std::vector<Ref<dynamic::SelectFrom>> selects;
    rfl::for_each(_union.selects_, [&](const auto& _select) {
        const auto s = to_table_or_query(_select);
        selects.push_back(std::get<Ref<dynamic::SelectFrom>>(s));
    });
    return dynamic::Union{ .selects_ = std::move(selects) };
}

}  // namespace sqlgen::transpilation

#endif
