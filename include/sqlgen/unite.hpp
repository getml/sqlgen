#ifndef SQLGEN_UNITE_HPP_
#define SQLGEN_UNITE_HPP_

#include "dynamic/Union.hpp"
#include "select_from.hpp"
#include "transpilation/to_union.hpp"

namespace sqlgen {

template <class ContainerType, class... Selects>
struct Union {
  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Ref<Connection>& _conn) const {
    const auto query = transpilation::to_union<ContainerType>(selects_);
    return _conn->template read<ContainerType>(query);
  }

  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Result<Ref<Connection>>& _res) const {
    return _res.and_then([&](const auto& _conn) { return (*this)(_conn); });
  }

  rfl::Tuple<Selects...> selects_;
};

template <class ContainerType, class... Selects>
auto unite(const Selects&... _selects) {
  return Union<ContainerType, Selects...>{rfl::Tuple<Selects...>(_selects...)};
}

}  // namespace sqlgen

#endif
