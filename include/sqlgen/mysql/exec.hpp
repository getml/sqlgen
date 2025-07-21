#ifndef SQLGEN_MYSQL_EXEC_HPP_
#define SQLGEN_MYSQL_EXEC_HPP_

#include <libpq-fe.h>

#include <rfl.hpp>
#include <string>

#include "../Ref.hpp"
#include "../Result.hpp"

namespace sqlgen::mysql {

Result<Ref<PGresult>> exec(const Ref<PGconn>& _conn,
                           const std::string& _sql) noexcept;

}  // namespace sqlgen::mysql

#endif
