#ifndef SQLGEN_EXEC_HPP_
#define SQLGEN_EXEC_HPP_

#include <rfl.hpp>

#include "Ref.hpp"
#include "Result.hpp"

namespace sqlgen {

inline Result<Ref<Connection>> exec(const Ref<Connection>& _conn,
                                    const std::string& _sql) {
  return _conn->execute(_sql).transform([&](const auto&) { return _conn; });
}

inline Result<Ref<Connection>> exec(const Result<Ref<Connection>>& _res,
                                    const std::string& _sql) {
  return _res.and_then([&](const auto& _conn) { return exec(_conn, _sql); });
}

struct Exec {
  Result<Ref<Connection>> operator()(const auto& _conn) const noexcept {
    try {
      return exec(_conn, sql_);
    } catch (std::exception& e) {
      return error(e.what());
    }
  }

  std::string sql_;
};

inline auto exec(const std::string& _sql) { return Exec{.sql_ = _sql}; }

};  // namespace sqlgen

#endif

