#ifndef SQLGEN_SELECT_FROM_HPP_
#define SQLGEN_SELECT_FROM_HPP_

#include <ranges>
#include <rfl.hpp>
#include <type_traits>

#include "Range.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "col.hpp"
#include "internal/GetColType.hpp"
#include "internal/is_range.hpp"
#include "is_connection.hpp"
#include "transpilation/fields_to_named_tuple_t.hpp"
#include "transpilation/to_select_from.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

template <class StructType, class FieldsType, class WhereType,
          class GroupByType, class OrderByType, class LimitType,
          class ContainerType, class Connection>
  requires is_connection<Connection>
auto select_from_impl(const Ref<Connection>& _conn, const FieldsType& _fields,
                      const WhereType& _where, const LimitType& _limit) {
  if constexpr (internal::is_range_v<ContainerType>) {
    const auto query =
        transpilation::to_select_from<StructType, FieldsType, WhereType,
                                      GroupByType, OrderByType, LimitType>(
            _fields, _where, _limit);
    return _conn->read(query).transform(
        [](auto&& _it) { return ContainerType(_it); });

  } else {
    const auto to_container = [](auto range) -> Result<ContainerType> {
      using ValueType = transpilation::value_t<ContainerType>;
      ContainerType container;
      for (auto& res : range) {
        if (res) {
          container.emplace_back(
              rfl::from_named_tuple<ValueType>(std::move(*res)));
        } else {
          return error(res.error().what());
        }
      }
      return container;
    };

    using RangeType =
        Range<transpilation::fields_to_named_tuple_t<StructType, FieldsType>>;

    return select_from_impl<StructType, FieldsType, WhereType, GroupByType,
                            OrderByType, LimitType, RangeType>(_conn, _fields,
                                                               _where, _limit)
        .and_then(to_container);
  }
}

template <class StructType, class FieldsType, class WhereType,
          class GroupByType, class OrderByType, class LimitType,
          class ContainerType, class Connection>
  requires is_connection<Connection>
auto select_from_impl(const Result<Ref<Connection>>& _res,
                      const FieldsType& _fields, const WhereType& _where,
                      const LimitType& _limit) {
  return _res.and_then([&](const auto& _conn) {
    return select_from_impl<StructType, FieldsType, WhereType, GroupByType,
                            OrderByType, LimitType, ContainerType>(
        _conn, _where, _limit);
  });
}

template <class StructType, class FieldsType, class WhereType = Nothing,
          class GroupByType = Nothing, class OrderByType = Nothing,
          class LimitType = Nothing, class ToType = Nothing>
struct SelectFrom {
  auto operator()(const auto& _conn) const {
    using ContainerType = std::conditional_t<
        std::is_same_v<ToType, Nothing>,
        Range<transpilation::fields_to_named_tuple_t<StructType, FieldsType>>,
        ToType>;
    return select_from_impl<StructType, FieldsType, WhereType, GroupByType,
                            OrderByType, LimitType, ContainerType>(
        _conn, fields_, where_, limit_);
  }

  FieldsType fields_;

  WhereType where_;

  LimitType limit_;
};

template <class StructType, class... FieldTypes>
inline auto select_from(const FieldTypes&... _fields) {
  using FieldsType =
      rfl::Tuple<typename internal::GetColType<FieldTypes>::Type...>;
  return SelectFrom<StructType, FieldsType>{
      .fields_ =
          FieldsType(internal::GetColType<FieldTypes>::get_value(_fields)...)};
}

}  // namespace sqlgen

#endif
