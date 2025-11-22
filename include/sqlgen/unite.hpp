#ifndef SQLGEN_UNITE_HPP_
#define SQLGEN_UNITE_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "Range.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "dynamic/Union.hpp"
#include "internal/is_range.hpp"
#include "internal/iterator_t.hpp"
#include "is_connection.hpp"
#include "transpilation/fields_to_named_tuple_t.hpp"
#include "transpilation/to_union.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

template <class ContainerType, class Connection, class... SelectTs>
  requires is_connection<Connection>
auto unite_impl(const Ref<Connection>& _conn,
                const rfl::Tuple<SelectTs...>& _selects) {
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

    using IteratorType = internal::iterator_t<
        transpilation::fields_to_named_tuple_t<rfl::Tuple<SelectTs...>>,
        decltype(_conn)>;

    using RangeType = Range<IteratorType>;

    return unite_impl<RangeType>(_conn, _selects).and_then(to_container);
  }
}

template <class _ContainerType, class... SelectTs>
struct Union {
  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Ref<Connection>& _conn) const {
    using ContainerType = std::conditional_t<
        std::is_same_v<std::remove_cvref_t<_ContainerType>, Nothing>,
        Range<internal::iterator_t<
            transpilation::fields_to_named_tuple_t<rfl::Tuple<SelectTs...>>,
            Connection>>,
        _ContainerType>;

    return unite_impl<ContainerType>(_conn, selects_);
  }

  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Result<Ref<Connection>>& _res) const {
    return _res.and_then([&](const auto& _conn) { return (*this)(_conn); });
  }

  rfl::Tuple<SelectTs...> selects_;
};

namespace transpilation {

template <class ContainerType, class... SelectTs>
struct ExtractTable<Union<ContainerType, SelectTs...>, false> {
  using Type = std::conditional_t<
      std::is_same_v<std::remove_cvref_t<ContainerType>, Nothing>,
      transpilation::fields_to_named_tuple_t<rfl::Tuple<SelectTs...>>,
      transpilation::value_t<ContainerType>>;
};

template <class ContainerType, class... SelectTs>
struct ToTableOrQuery<Union<ContainerType, SelectTs...>> {
  dynamic::SelectFrom::TableOrQueryType operator()(const auto& _query) {
    return transpilation::to_union<ContainerType>(_query.selects_);
  }
};

}  // namespace transpilation

template <class ContainerType, class... SelectTs>
auto unite(const SelectTs&... _selects) {
  return Union<ContainerType, SelectTs...>{
      rfl::Tuple<SelectTs...>(_selects...)};
}

template <class... SelectTs>
auto unite(const SelectTs&... _selects) {
  return unite<Nothing>(_selects...);
}

}  // namespace sqlgen

#endif
