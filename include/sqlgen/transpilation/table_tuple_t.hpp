#ifndef SQLGEN_TRANSPILATION_TABLETUPLET_HPP_
#define SQLGEN_TRANSPILATION_TABLETUPLET_HPP_

#include <rfl.hpp>
#include <type_traits>
#include <utility>

#include "../Literal.hpp"
#include "../Result.hpp"

namespace sqlgen::transpilation {

template <class StructType, class AliasType, class JoinsType>
struct TableTupleType;

template <class StructType>
struct TableTupleType<StructType, Nothing, Nothing> {
  using Type = StructType;
};

template <class StructType, class AliasType, class... JoinTypes>
struct TableTupleType<StructType, AliasType, rfl::Tuple<JoinTypes...>> {
  using Type = rfl::Tuple<std::pair<StructType, AliasType>,
                          std::pair<typename JoinTypes::TableOrQueryType,
                                    typename JoinTypes::Alias>...>;
};

template <class StructType, class AliasType, class JoinsType>
using table_tuple_t =
    typename TableTupleType<std::remove_cvref_t<StructType>,
                            std::remove_cvref_t<AliasType>,
                            std::remove_cvref_t<JoinsType>>::Type;

}  // namespace sqlgen::transpilation

#endif
