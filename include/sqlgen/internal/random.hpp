#include <random>
#include <string>

namespace sqlgen::internal {

/// Generates a random number to be used in the names of iterators and such.
inline std::string random() {
  std::random_device rd;
  std::mt19937 gen(rd());
  return std::to_string(gen());
}

}  // namespace sqlgen::internal
