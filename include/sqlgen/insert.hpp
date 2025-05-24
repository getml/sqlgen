#ifndef SQLGEN_INSERT_HPP_
#define SQLGEN_INSERT_HPP_

#include <functional>
#include <iterator>
#include <optional>
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
Result<Ref<Connection>> insert(const Ref<Connection>& _conn, ItBegin _begin,
                               ItEnd _end) {
  using T =
      std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;

  const auto insert_stmt =
      transpilation::to_insert_or_write<T, dynamic::Insert>();

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
Result<Ref<Connection>> insert(const Result<Ref<Connection>>& _res,
                               ItBegin _begin, ItEnd _end) {
  return _res.and_then(
      [&](const auto& _conn) { return insert(_conn, _begin, _end); });
}

template <class ContainerType>
auto insert(const auto& _conn, const ContainerType& _data) {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    return insert(_conn, _data.begin(), _data.end());
  } else {
    return insert(_conn, &_data, &_data + 1);
  }
}

template <class ContainerType>
auto insert(const auto& _conn,
            const std::reference_wrapper<ContainerType>& _data) {
  return insert(_conn, _data.get());
}

template <class ContainerType>
struct Insert {
  auto operator()(const auto& _conn) const { return insert(_conn, data_); }

  ContainerType data_;
};

template <class ContainerType>
Insert<ContainerType> insert(const ContainerType& _data) {
  return Insert<ContainerType>{.data_ = _data};
}

};  // namespace sqlgen

#endif

