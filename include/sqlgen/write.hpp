#ifndef SQLGEN_WRITE_HPP_
#define SQLGEN_WRITE_HPP_

#include <iterator>
#include <optional>
#include <ranges>
#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <vector>

#include "Connection.hpp"
#include "Ref.hpp"
#include "Result.hpp"
#include "dynamic/Write.hpp"
#include "internal/batch_size.hpp"
#include "internal/to_str_vec.hpp"
#include "transpilation/to_create_table.hpp"
#include "transpilation/to_insert_or_write.hpp"

namespace sqlgen {

template <class ItBegin, class ItEnd>
Result<Ref<Connection>> write(const Ref<Connection>& _conn, ItBegin _begin,
                              ItEnd _end) noexcept {
  using T =
      std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;

  const auto start_write = [&](const auto&) -> Result<Nothing> {
    const auto write_stmt =
        transpilation::to_insert_or_write<T, dynamic::Write>();
    return _conn->start_write(write_stmt);
  };

  const auto write = [&](const auto&) -> Result<Nothing> {
    try {
      std::vector<std::vector<std::optional<std::string>>> data;
      for (auto it = _begin; it != _end; ++it) {
        data.emplace_back(internal::to_str_vec(*it));
        if (data.size() == SQLGEN_BATCH_SIZE) {
          _conn->write(data).value();
          data.clear();
        }
      }
      if (data.size() != 0) {
        _conn->write(data).value();
      }
      return Nothing{};
    } catch (std::exception& e) {
      _conn->end_write();
      return error(e.what());
    }
  };

  const auto end_write = [&](const auto&) -> Result<Nothing> {
    return _conn->end_write();
  };

  const auto create_table_stmt = transpilation::to_create_table<T>();

  return _conn->execute(_conn->to_sql(create_table_stmt))
      .and_then(start_write)
      .and_then(write)
      .and_then(end_write)
      .transform([&](const auto&) { return _conn; });
}

template <class ItBegin, class ItEnd>
auto write(const Result<Ref<Connection>>& _res, ItBegin _begin,
           ItEnd _end) noexcept {
  return _res.and_then(
      [&](const auto& _conn) { return write(_conn, _begin, _end); });
}

template <class ConnectionType, class ContainerType>
auto write(const ConnectionType& _conn,
           const ContainerType& _container) noexcept {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    return write(_conn, _container.begin(), _container.end());
  } else {
    return write(_conn, &_container, &_container + 1);
  }
}

}  // namespace sqlgen

#endif
