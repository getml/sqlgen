#include "sqlgen/internal/strings/strings.hpp"

namespace sqlgen::internal::strings {

char to_lower(const char ch) {
  if (ch >= 'A' && ch <= 'Z') {
    return ch + ('a' - 'A');
  } else {
    return ch;
  }
}

std::string to_lower(const std::string& _str) {
  auto str = _str;
  for (char& ch : str) {
    ch = to_lower(ch);
  }
  return str;
}

char to_upper(const char ch) {
  if (ch >= 'a' && ch <= 'z') {
    return ch + ('A' - 'a');
  } else {
    return ch;
  }
}

std::string to_upper(const std::string& _str) {
  auto str = _str;
  for (char& ch : str) {
    ch = to_upper(ch);
  }
  return str;
}

std::string join(const std::string& _delimiter,
                 const std::vector<std::string>& _strings) {
  if (_strings.size() == 0) {
    return "";
  }
  auto res = _strings[0];
  for (size_t i = 1; i < _strings.size(); ++i) {
    res += _delimiter + _strings[i];
  }
  return res;
}

std::string replace_all(const std::string& _str, const std::string& _from,
                        const std::string& _to) {
  auto str = _str;

  size_t pos = 0;
  while ((pos = str.find(_from, pos)) != std::string::npos) {
    str.replace(pos, _from.length(), _to);
    pos += _to.length();
  }
  return str;
}

std::vector<std::string> split(const std::string& _str,
                               const std::string& _delimiter) {
  auto str = _str;
  size_t pos = 0;
  std::vector<std::string> result;
  while ((pos = str.find(_delimiter)) != std::string::npos) {
    result.emplace_back(str.substr(0, pos));
    str.erase(0, pos + _delimiter.length());
  }
  result.emplace_back(std::move(str));
  return result;
}

}  // namespace sqlgen::internal::strings
