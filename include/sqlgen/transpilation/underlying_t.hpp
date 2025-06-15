#ifndef SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_
#define SQLGEN_TRANSPILATION_UNDERLYINGT_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "Col.hpp"
#include "Desc.hpp"
#include "Operation.hpp"
#include "Value.hpp"
#include "all_columns_exist.hpp"
#include "dynamic_operator_t.hpp"
#include "remove_reflection_t.hpp"

namespace sqlgen::transpilation {

template <class T, class _Type>
struct Underlying;

template <class T, rfl::internal::StringLiteral _name>
struct Underlying<T, Col<_name>> {
  static_assert(all_columns_exist<T, Col<_name>>(), "All columns must exist.");
  using Type = remove_reflection_t<rfl::field_type_t<_name, T>>;
};

template <class T, rfl::internal::StringLiteral _name>
struct Underlying<T, Desc<Col<_name>>> {
  using Type = remove_reflection_t<rfl::field_type_t<_name, T>>;
};

template <class T, Operator _op, class Operand1Type, class Operand2Type>
  requires((num_operands_v<_op>) == 2)
struct Underlying<T, Operation<_op, Operand1Type, Operand2Type>> {
  using Underlying1 =
      typename Underlying<T, std::remove_cvref_t<Operand1Type>>::Type;
  using Underlying2 =
      typename Underlying<T, std::remove_cvref_t<Operand2Type>>::Type;

  static_assert(
      requires(Underlying1 op1, Underlying2 op2) { op1 + op2; },
      "Binary operations are not possible on these types.");

  using Type =
      std::invoke_result_t<decltype([](const auto& op1, const auto& op2) {
                             return op1 + op2;
                           }),
                           Underlying1, Underlying2>;
};

template <class T, class _Type>
struct Underlying<T, Value<_Type>> {
  using Type = _Type;
};

template <class T, class U>
using underlying_t =
    typename Underlying<std::remove_cvref_t<T>, std::remove_cvref_t<U>>::Type;

}  // namespace sqlgen::transpilation

#endif
