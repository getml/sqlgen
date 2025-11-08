#ifndef SQLGEN_PARSING_PARSER_UNIQUE_HPP_
#define SQLGEN_PARSING_PARSER_UNIQUE_HPP_

#include <string>
#include <type_traits>

#include "../Result.hpp"
#include "../Unique.hpp"
#include "../dynamic/Type.hpp"
#include "Parser_base.hpp"
#include "RawType.hpp"

namespace sqlgen::parsing {

template <class T, RawType _raw_type>
struct Parser<Unique<T>, _raw_type> {
  static Result<Unique<T>> read(
      const std::optional<std::string>& _str) noexcept {
    return Parser<std::remove_cvref_t<T>, _raw_type>::read(_str).transform(
        [](auto&& _t) { return Unique<T>(std::move(_t)); });
  }

  static std::optional<std::string> write(const Unique<T>& _f) noexcept {
    return Parser<std::remove_cvref_t<T>, _raw_type>::write(_f.value());
  }

  static dynamic::Type to_type() noexcept {
    return Parser<std::remove_cvref_t<T>, _raw_type>::to_type().visit(
        [](auto _t) -> dynamic::Type {
          _t.properties.unique = true;
          return _t;
        });
  }
};

}  // namespace sqlgen::parsing

#endif
