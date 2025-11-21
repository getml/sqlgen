#ifndef SQLGEN_UNITE_HPP_
#define SQLGEN_UNITE_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "Range.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "dynamic/Union.hpp"
#include "internal/all_same_v.hpp"
#include "internal/is_range.hpp"
#include "internal/iterator_t.hpp"
#include "is_connection.hpp"
#include "transpilation/fields_to_named_tuple_t.hpp"
#include "transpilation/table_tuple_t.hpp"
#include "transpilation/to_union.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

template <class ContainerType, class Connection, class... Selects>
  requires is_connection<Connection>
auto unite_impl(const Ref<Connection>& _conn,
                const rfl::Tuple<Selects...>& _selects) {
  if constexpr (internal::is_range_v<ContainerType>) {
    const auto query = transpilation::to_union<ContainerType>(_selects);
    return _conn->template read<ContainerType>(query);

  } else {
    const auto to_container = [](auto range) -> Result<ContainerType> {
      using ValueType = transpilation::value_t<ContainerType>;
      ContainerType container;
      for (auto& res : range) {
        if (res) {
          container.emplace_back(
              rfl::from_named_tuple<ValueType>(std::move(*res)));
        } else {
          return error("One of the results in the union was an error.");
        }
      }
      return container;
    };

    using NamedTupleTypes = rfl::Tuple<transpilation::fields_to_named_tuple_t<
        transpilation::table_tuple_t<typename Selects::TableOrQueryType,
                                     typename Selects::AliasType,
                                     typename Selects::JoinsType>,
        typename Selects::FieldsType>...>;

    static_assert(
        internal::all_same_v<NamedTupleTypes>,
        "All SELECT statements in a UNION must return the same columns with "
        "the same types.");

    using IteratorType =
        internal::iterator_t<rfl::tuple_element_t<0, NamedTupleTypes>,
                             decltype(_conn)>;

    using RangeType = Range<IteratorType>;

    return unite_impl<RangeType>(_conn, _selects).and_then(to_container);
  }
}

template <class ContainerType, class... Selects>
struct Union {
  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Ref<Connection>& _conn) const {
    return unite_impl<ContainerType>(_conn, selects_);
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
