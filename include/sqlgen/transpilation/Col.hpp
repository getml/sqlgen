#ifndef SQLGEN_TRANSPILATION_COL_HPP_
#define SQLGEN_TRANSPILATION_COL_HPP_

#include <rfl.hpp>
#include <string>

namespace sqlgen::transpilation {

template <rfl::internal::StringLiteral _name>
struct Col {
  using ColType = Col<_name>;
  using Name = rfl::Literal<_name>;

  /// Returns the column name.
  std::string name() const noexcept { return Name().str(); }
};

}  // namespace sqlgen::transpilation

#endif
