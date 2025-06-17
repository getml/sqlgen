#ifndef SQLGEN_OPERATIONS_HPP_
#define SQLGEN_OPERATIONS_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "transpilation/Operation.hpp"
#include "transpilation/Operator.hpp"

namespace sqlgen {

template <class T>
auto abs(const T& _t) {
  return transpilation::Operation<transpilation::Operator::abs,
                                  std::remove_cvref_t<T>>{.operand1 = _t};
}

}  // namespace sqlgen

#endif
