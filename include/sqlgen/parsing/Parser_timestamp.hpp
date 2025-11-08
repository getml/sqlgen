#ifndef SQLGEN_PARSING_PARSER_TIMESTAMP_HPP_
#define SQLGEN_PARSING_PARSER_TIMESTAMP_HPP_

#include <string>
#include <type_traits>

#include "../Result.hpp"
#include "../Timestamp.hpp"
#include "../dynamic/Type.hpp"
#include "../dynamic/types.hpp"
#include "Parser_base.hpp"
#include "Parser_default.hpp"
#include "RawType.hpp"

namespace sqlgen::parsing {

template <rfl::internal::StringLiteral _format, RawType _raw_type>
struct Parser<Timestamp<_format>, _raw_type> {
  using TSType = Timestamp<_format>;

  static Result<TSType> read(const std::optional<std::string>& _str) noexcept {
    return Parser<std::string, _raw_type>::read(_str).and_then(
        [](auto&& _s) -> Result<TSType> {
          return TSType::from_string(std::move(_s));
        });
  }

  static std::optional<std::string> write(const TSType& _t) noexcept {
    return Parser<std::string, _raw_type>::write(_t.str());
  }

  static dynamic::Type to_type() noexcept {
    const std::string format = typename TSType::Format().str();
    if (format.find("%z") != std::string::npos) {
      return dynamic::types::TimestampWithTZ{};
    } else if (format.find("%H") != std::string::npos ||
               format.find("%M") != std::string::npos ||
               format.find("%S") != std::string::npos ||
               format.find("%R") != std::string::npos ||
               format.find("%T") != std::string::npos) {
      return dynamic::types::Timestamp{};
    } else {
      return dynamic::types::Date{};
    }
  }
};

}  // namespace sqlgen::parsing

#endif
