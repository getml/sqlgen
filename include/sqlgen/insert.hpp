#ifndef SQLGEN_INSERT_HPP_
#define SQLGEN_INSERT_HPP_

#include <functional>
#include <iterator>
#include <optional>
#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "internal/batch_size.hpp"
#include "internal/has_primary_key.hpp"
#include "internal/to_str_vec.hpp"
#include "is_connection.hpp"
#include "transpilation/to_insert_or_write.hpp"

namespace sqlgen {

template <class ItBegin, class ItEnd, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> insert_impl(const Ref<Connection>& _conn,
                                    ItBegin _begin, ItEnd _end,
                                    bool _or_replace) {
  using T =
      std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;

  const auto insert_stmt =
      transpilation::to_insert_or_write<T, dynamic::Insert>(_or_replace);

  std::vector<std::vector<std::optional<std::string>>> data;

  for (auto it = _begin; it != _end; ++it) {
    data.emplace_back(internal::to_str_vec(*it));
    if (data.size() == SQLGEN_BATCH_SIZE) {
      const auto res = _conn->insert(insert_stmt, data);
      if (!res) {
        return error(res.error().what());
      }
      data.clear();
    }
  }

  if (data.size() != 0) {
    const auto res = _conn->insert(insert_stmt, data);
    if (!res) {
      return error(res.error().what());
    }
  }

  return _conn;
}

template <class ItBegin, class ItEnd, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> insert_impl(const Result<Ref<Connection>>& _res,
                                    ItBegin _begin, ItEnd _end,
                                    bool _or_replace) {
  return _res.and_then([&](const auto& _conn) {
    return insert_impl(_conn, _begin, _end, _or_replace);
  });
}

template <class ContainerType>
auto insert_impl(const auto& _conn, const ContainerType& _data,
                 bool _or_replace) {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    return insert_impl(_conn, _data.begin(), _data.end(), _or_replace);
  } else {
    return insert_impl(_conn, &_data, &_data + 1, _or_replace);
  }
}

template <class ContainerType>
auto insert_impl(const auto& _conn,
                 const std::reference_wrapper<ContainerType>& _data,
                 bool _or_replace) {
  return insert_impl(_conn, _data.get(), _or_replace);
}

template <class ContainerType>
struct Insert {
  auto operator()(const auto& _conn) const {
    return insert_impl(_conn, data_, or_replace_);
  }

  ContainerType data_;
  bool or_replace_;
};

template <class ContainerType>
Insert<ContainerType> insert_impl(const ContainerType& _data,
                                  bool _or_replace) {
  return Insert<ContainerType>{.data_ = _data, .or_replace_ = _or_replace};
}

template <class... Args>
auto insert(const Args&... args) {
  return insert_impl(args..., false);
}

template <class ContainerType>
auto insert_or_replace(const auto& _conn, const ContainerType& _data) {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    static_assert(
        internal::has_primary_key_v<typename ContainerType::value_type>,
        "The table must have a primary key for insert_or_replace(...) to "
        "work.");
  } else {
    static_assert(internal::has_primary_key_v<ContainerType>,
                  "The table must have a primary key for "
                  "insert_or_replace(...) to work.");
  }
  return insert_impl(_conn, _data, true);
}

template <class ContainerType>
auto insert_or_replace(const ContainerType& _data) {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    static_assert(
        internal::has_primary_key_v<typename ContainerType::value_type>,
        "The table must have a primary key for insert_or_replace(...) to "
        "work.");
  } else {
    static_assert(internal::has_primary_key_v<ContainerType>,
                  "The table must have a primary key for "
                  "insert_or_replace(...) to work.");
  }
  return insert_impl(_data, true);
}

};  // namespace sqlgen

#endif

