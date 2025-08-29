#ifndef SQLGEN_INSERT_HPP_
#define SQLGEN_INSERT_HPP_

#include <functional>
#include <iterator>
#include <optional>
#include <utility>
#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <vector>

#include "internal/batch_size.hpp"
#include "internal/to_str_vec.hpp"
#include "is_connection.hpp"
#include "transpilation/to_insert_or_write.hpp"

namespace sqlgen {

template <class ItBegin, class ItEnd, class Connection>
  requires is_connection<Connection>
Result<Ref<Connection>> insert_impl(const Ref<Connection>& _conn, ItBegin _begin,
                               ItEnd _end, bool or_replace) {
  using T =
      std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;

  const auto insert_stmt =
      transpilation::to_insert_or_write<T, dynamic::Insert>(or_replace);

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
                               ItBegin _begin, ItEnd _end, bool or_replace) {
  return _res.and_then(
      [&](const auto& _conn) { return insert_impl(_conn, _begin, _end, or_replace); });
}

template <class ContainerType>
auto insert_impl(const auto& _conn, const ContainerType& _data, bool or_replace) {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    return insert_impl(_conn, _data.begin(), _data.end(), or_replace);
  } else {
    return insert_impl(_conn, &_data, &_data + 1, or_replace);
  }
}

template <class ContainerType>
auto insert_impl(const auto& _conn,
            const std::reference_wrapper<ContainerType>& _data, bool or_replace) {
  return insert_impl(_conn, _data.get(), or_replace);
}

template <class ContainerType>
struct Insert {
  auto operator()(const auto& _conn) const { return insert_impl(_conn, data_, or_replace); }

  ContainerType data_;
  bool or_replace;
};

template <class ContainerType>
Insert<ContainerType> insert_impl(const ContainerType& _data, bool or_replace) {
  return Insert<ContainerType>{.data_ = _data, .or_replace = or_replace};
}

template <class... Args>
auto insert(Args&&... args) {
  return insert_impl(std::forward<Args>(args)..., false);
}

template <class... Args>
auto insert_or_replace(Args&&... args) {
  return insert_impl(std::forward<Args>(args)..., true);
}

};  // namespace sqlgen

#endif

