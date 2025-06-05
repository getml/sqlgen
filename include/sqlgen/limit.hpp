#ifndef SQLGEN_LIMIT_HPP_
#define SQLGEN_LIMIT_HPP_

#include <type_traits>

#include "Result.hpp"
#include "read.hpp"
#include "transpilation/Limit.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

using Limit = transpilation::Limit;

template <class ContainerType, class WhereType, class OrderByType,
          class LimitType>
auto operator|(const Read<ContainerType, WhereType, OrderByType, LimitType>& _r,
               const Limit& _limit) {
  static_assert(std::is_same_v<LimitType, Nothing>,
                "You cannot call limit twice (but you can order by more "
                "than one column).");
  return Read<ContainerType, WhereType, OrderByType, Limit>{.where_ = _r.where_,
                                                            .limit_ = _limit};
}

inline auto limit(const size_t _val) { return Limit{_val}; };

}  // namespace sqlgen

#endif
