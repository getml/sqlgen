#ifndef SQLGEN_MYSQL_EXEC_HPP_
#define SQLGEN_MYSQL_EXEC_HPP_

#include <mysql.h>

#include <string>

#include "../Ref.hpp"
#include "../Result.hpp"

namespace sqlgen::mysql {

Result<Nothing> exec(const Ref<MYSQL>& _conn, const std::string& _sql) noexcept;

}  // namespace sqlgen::mysql

#endif
