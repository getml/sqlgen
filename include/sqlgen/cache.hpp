#ifndef SQLGEN_CACHE_HPP_
#define SQLGEN_CACHE_HPP_

#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "Ref.hpp"
#include "Result.hpp"
#include "internal/query_value_t.hpp"
#include "is_connection.hpp"
#include "transpilation/to_sql.hpp"

namespace sqlgen {

template <class QueryT, class Connection, size_t _max_size>
  requires is_connection<Connection>
class CacheImpl {
 public:
  using ValueType = internal::query_value_t<QueryT, Ref<Connection>>;

  static Result<ValueType> fetch(const QueryT& _query,
                                 const Ref<Connection>& _conn) {
    const auto sql = _conn->to_sql(transpilation::to_sql(_query));
    const auto it = cache_.find(sql);
    if (it != cache_.end()) {
      return it->second;
    }
    return _query(_conn).transform([&](auto&& _val) {
      cache_[sql] = _val;
      return _val;
    });
  }

  static const std::unordered_map<std::string, ValueType>& cache() {
    return cache_;
  }

 private:
  inline static std::unordered_map<std::string, ValueType> cache_;
};

template <class QueryT, size_t _max_size>
struct Cache {
  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Ref<Connection>& _conn) const {
    return CacheImpl<QueryT, std::remove_cvref_t<Connection>, _max_size>::fetch(
        query_, _conn);
  }

  template <class Connection>
    requires is_connection<Connection>
  auto operator()(const Result<Ref<Connection>>& _res) const {
    return _res.and_then(
        [&](const Ref<Connection>& _conn) { return (*this)(_conn); });
  }

  template <class Connection>
    requires is_connection<Connection>
  static const auto& cache(const Ref<Connection>& _conn) {
    return CacheImpl<QueryT, std::remove_cvref_t<Connection>,
                     _max_size>::cache();
  }

  template <class Connection>
    requires is_connection<Connection>
  static const auto& cache(const Result<Ref<Connection>>& _res) {
    return CacheImpl<QueryT, std::remove_cvref_t<Connection>,
                     _max_size>::cache();
  }

  QueryT query_;
};

template <size_t _max_size, class QueryT>
auto cache(const QueryT& _query) {
  return Cache<std::remove_cvref_t<QueryT>, _max_size>{.query_ = _query};
}

}  // namespace sqlgen

#endif
