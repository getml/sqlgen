#ifndef SQLGEN_INTERNAL_STRINGS_STRINGS_HPP_
#define SQLGEN_INTERNAL_STRINGS_STRINGS_HPP_

#include <string>
#include <vector>

namespace sqlgen::internal::strings {

char to_lower(const char ch);

std::string to_lower(const std::string& _str);

char to_upper(const char ch);

std::string to_upper(const std::string& _str);

std::string join(const std::string& _delimiter,
                 const std::vector<std::string>& _strings);

std::string replace_all(const std::string& _str, const std::string& _from,
                        const std::string& _to);

std::vector<std::string> split(const std::string& _str,
                               const std::string& _delimiter);

}  // namespace sqlgen::internal::strings

#endif
