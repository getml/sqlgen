#ifndef SQLGEN_UNION_HPP_
#define SQLGEN_UNION_HPP_

#include "select_from.hpp"
#include "dynamic/Union.hpp"
#include "transpilation/to_union.hpp"

namespace sqlgen {

template <class... Selects>
struct Union {
  auto operator()(const auto& _conn) const {
    const auto query = transpilation::to_union(*this);
    using FirstSelect = std::remove_cvref_t<decltype(rfl::get<0>(selects_))>;
    using ModelType = typename FirstSelect::ModelType;
    using ContainerType = std::vector<ModelType>;
    return _conn->template read<ContainerType>(query);
  }

  auto operator()(const Result<Ref<auto>>& _res) const {
    return _res.and_then([&](const auto& _conn) { return (*this)(_conn); });
  }

  rfl::Tuple<Selects...> selects_;
};

template <class... Selects>
auto union_(const Selects&... _selects) {
  return Union<Selects...>{rfl::Tuple<Selects...>(_selects...)};
}

}  // namespace sqlgen

#endif
