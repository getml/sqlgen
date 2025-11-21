#ifndef SQLGEN_POSTGRES_POSTGRESV2CONNECTION_HPP_
#define SQLGEN_POSTGRES_POSTGRESV2CONNECTION_HPP_

#include <libpq-fe.h>

#include <memory>
#include <rfl.hpp>
#include <stdexcept>

#include "../Ref.hpp"
#include "../Result.hpp"
#include "../sqlgen_api.hpp"

namespace sqlgen::postgres {

class SQLGEN_API PostgresV2Connection {
 public:
  PostgresV2Connection(PGconn* _ptr)
      : ptr_(Ref<PGconn>::make(std::shared_ptr<PGconn>(_ptr, &PQfinish))
                 .value()) {}

  ~PostgresV2Connection() = default;

  static rfl::Result<PostgresV2Connection> make(
      const std::string& _conn_str) noexcept;

  PGconn* ptr() const { return ptr_.get(); }

 private:
  Ref<PGconn> ptr_;
};

}  // namespace sqlgen::postgres

#endif
