#ifndef SQLGEN_CACHE_HPP_
#define SQLGEN_CACHE_HPP_

#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "Ref.hpp"
#include "Result.hpp"
#include "is_connection.hpp"

namespace sqlgen {

template <class QueryT, size_t _max_size>
class CacheImpl {
 public:
  using ValueType = ...;  // TODO;

  template <class Connection>
    requires is_connection<Connection>
  static auto fetch(const QueryT& _query, const Ref<Connection>& _conn) {
    const auto sql = _conn->to_sql(_query);
    const auto it = results_.find(sql);
    if (it != results_.end()) {
      return it->second;
    }
    return _query(_conn).transform([&](auto&& _val) {
      results_[sql] = _val;
      return _val;
    });
  }

 private:
  static std::unordered_map<std::string, ValueType> results_;
};

template <class QueryT, size_t _max_size>
struct Cache {
  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Ref<Connection>& _conn) {
    return CacheImpl<QueryT, _max_size>::fetch(query_, _conn);
  }

  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Result<Ref<Connection>>& _res) {
    return _res.and_then(
        [&](const Ref<Connection>& _conn) { return (*this)(_conn); });
  }

  Query query_;
};

template <size_t _max_size, class QueryT>
auto cache(const QueryT& _query) {
  return Cache<std::remove_cvref_t<QueryT>, _max_size>{.query = _query};
}

}  // namespace sqlgen

#endif
