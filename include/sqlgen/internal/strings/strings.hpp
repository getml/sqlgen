#ifndef SQLGEN_INTERNAL_STRINGS_STRINGS_HPP_
#define SQLGEN_INTERNAL_STRINGS_STRINGS_HPP_

#include <string>
#include <vector>

#include "../../sqlgen_api.hpp"

namespace sqlgen::internal::strings {

char SQLGEN_API to_lower(const char ch);

std::string SQLGEN_API to_lower(const std::string& _str);

char SQLGEN_API to_upper(const char ch);

std::string SQLGEN_API to_upper(const std::string& _str);

std::string SQLGEN_API join(const std::string& _delimiter,
                            const std::vector<std::string>& _strings);

std::string SQLGEN_API replace_all(const std::string& _str,
                                   const std::string& _from,
                                   const std::string& _to);

std::vector<std::string> SQLGEN_API split(const std::string& _str,
                                          const std::string& _delimiter);

std::string SQLGEN_API ltrim(const std::string& _str,
                             const std::string& _chars = " ");

std::string SQLGEN_API rtrim(const std::string& _str,
                             const std::string& _chars = " ");

inline std::string trim(const std::string& _str,
                        const std::string& _chars = " ") {
  return ltrim(rtrim(_str, _chars), _chars);
}

}  // namespace sqlgen::internal::strings

#endif
