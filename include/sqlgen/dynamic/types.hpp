#ifndef SQLGEN_DYNAMIC_TABLE_TYPES_HPP_
#define SQLGEN_DYNAMIC_TABLE_TYPES_HPP_

#include <cstdint>
#include <string>

namespace sqlgen::dynamic::types {

struct Properties {
  bool primary = false;
  bool nullable = false;
};

// To be used as the default value.
struct Unknown {
  Properties properties;
};

struct Boolean {
  Properties properties;
};

struct Float32 {
  Properties properties;
};

struct Float64 {
  Properties properties;
};

struct Int8 {
  Properties properties;
};

struct Int16 {
  Properties properties;
};

struct Int32 {
  Properties properties;
};

struct Int64 {
  Properties properties;
};

struct Text {
  Properties properties;
};

struct Timestamp {
  std::string tz;
  Properties properties;
};

struct TimestampWithTZ {
  std::string tz;
  Properties properties;
};

struct VarChar {
  uint16_t length;
  Properties properties;
};

}  // namespace sqlgen::dynamic::types

#endif
