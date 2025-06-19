#ifndef SQLGEN_OPERATIONS_HPP_
#define SQLGEN_OPERATIONS_HPP_

#include <rfl.hpp>
#include <type_traits>

#include "col.hpp"
#include "transpilation/Operation.hpp"
#include "transpilation/Operator.hpp"
#include "transpilation/to_transpilation_type.hpp"

namespace sqlgen {

template <class T>
auto abs(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::abs, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class TargetType, class T>
auto cast(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<
      transpilation::Operator::cast, Type,
      transpilation::TypeHolder<std::remove_cvref_t<TargetType>>>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto ceil(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::ceil, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T, class... Ts>
auto concat(const T& _t, const Ts&... _ts) {
  using Type = rfl::Tuple<
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type,
      typename transpilation::ToTranspilationType<
          std::remove_cvref_t<Ts>>::Type...>;
  return transpilation::Operation<transpilation::Operator::concat, Type>{
      .operand1 = Type(transpilation::to_transpilation_type(_t),
                       transpilation::to_transpilation_type(_ts)...)};
}

template <class T>
auto cos(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::cos, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto exp(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::exp, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto floor(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::floor, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto length(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::length, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto ln(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::ln, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto log2(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::log2, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto lower(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::lower, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T, class U>
auto ltrim(const T& _t, const U& _u) {
  using Type1 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  using Type2 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<U>>::Type;
  return transpilation::Operation<transpilation::Operator::ltrim, Type1, Type2>{
      .operand1 = transpilation::to_transpilation_type(_t),
      .operand2 = transpilation::to_transpilation_type(_u)};
}

template <class T>
auto ltrim(const T& _t) {
  return ltrim(_t, std::string(" "));
}

template <class T, class U>
auto round(const T& _t, const U& _u) {
  using Type1 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  using Type2 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<U>>::Type;
  return transpilation::Operation<transpilation::Operator::round, Type1, Type2>{
      .operand1 = transpilation::to_transpilation_type(_t),
      .operand2 = transpilation::to_transpilation_type(_u)};
}

template <class T, class U>
auto rtrim(const T& _t, const U& _u) {
  using Type1 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  using Type2 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<U>>::Type;
  return transpilation::Operation<transpilation::Operator::rtrim, Type1, Type2>{
      .operand1 = transpilation::to_transpilation_type(_t),
      .operand2 = transpilation::to_transpilation_type(_u)};
}

template <class T>
auto rtrim(const T& _t) {
  return rtrim(_t, std::string(" "));
}

template <class T>
auto sin(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::sin, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto sqrt(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::sqrt, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T>
auto tan(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::tan, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

template <class T, class U>
auto trim(const T& _t, const U& _u) {
  using Type1 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  using Type2 =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<U>>::Type;
  return transpilation::Operation<transpilation::Operator::trim, Type1, Type2>{
      .operand1 = transpilation::to_transpilation_type(_t),
      .operand2 = transpilation::to_transpilation_type(_u)};
}

template <class T>
auto trim(const T& _t) {
  return trim(_t, std::string(" "));
}

template <class T>
auto upper(const T& _t) {
  using Type =
      typename transpilation::ToTranspilationType<std::remove_cvref_t<T>>::Type;
  return transpilation::Operation<transpilation::Operator::upper, Type>{
      .operand1 = transpilation::to_transpilation_type(_t)};
}

}  // namespace sqlgen

#endif
