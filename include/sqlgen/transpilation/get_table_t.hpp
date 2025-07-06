#ifndef SQLGEN_TRANSPILATION_GETTABLET_HPP_
#define SQLGEN_TRANSPILATION_GETTABLET_HPP_

#include <rfl.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../Literal.hpp"
#include "../dynamic/JoinType.hpp"

namespace sqlgen::transpilation {

template <class Alias, class _TableType, class _AliasType>
struct PairWrapper {
  using TableType = _TableType;
  using AliasType = _AliasType;

  template <class OtherTableType, class OtherAliasType>
  friend consteval auto operator||(
      const PairWrapper&,
      const PairWrapper<Alias, OtherTableType, OtherAliasType>&) noexcept {
    if constexpr (std::is_same_v<Alias, std::remove_cvref_t<OtherAliasType>>) {
      return PairWrapper<Alias, OtherTableType, OtherAliasType>{};
    } else {
      return PairWrapper{};
    }
  }
};

template <class Alias, class T>
struct GetTableType;

template <class Alias, class... TableTypes, class... AliasTypes>
struct GetTableType<Alias, rfl::Tuple<std::pair<TableTypes, AliasTypes>...>> {
  static constexpr auto wrapper =
      (PairWrapper<Alias, Nothing, Nothing>{} || ... ||
       PairWrapper<Alias, TableTypes, AliasTypes>{});

  using TableType = std::remove_cvref_t<typename decltype(wrapper)::TableType>;

  static_assert(!std::is_same_v<TableType, Nothing>,
                "Alias could not be identified.");
};

template <class T>
struct GetTableType<Literal<"">, T> {
  using TableType = T;
};

template <size_t _i, class... TableTypes, class... AliasTypes>
struct GetTableType<std::integral_constant<size_t, _i>,
                    rfl::Tuple<std::pair<TableTypes, AliasTypes>...>> {
  using TableType = typename std::tuple_element_t<
      _i, rfl::Tuple<std::pair<TableTypes, AliasTypes>...>>::first_type;

  static_assert(!std::is_same_v<TableType, Nothing>,
                "Alias could not be identified.");
};

template <class T, size_t _i>
struct GetTableType<std::integral_constant<size_t, _i>, T> {
  using TableType = T;
};

template <class Alias, class T>
using get_table_t = typename GetTableType<std::remove_cvref_t<Alias>,
                                          std::remove_cvref_t<T>>::TableType;

}  // namespace sqlgen::transpilation

#endif
