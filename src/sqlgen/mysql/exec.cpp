#include "sqlgen/mysql/exec.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

#include "sqlgen/mysql/make_error.hpp"

namespace sqlgen::mysql {

Result<Ref<MYSQL_RES>> exec(const Ref<MYSQL>& _conn,
                            const std::string& _sql) noexcept {
  const auto err = mysql_real_query(_conn.get(), _sql.c_str(),
                                    static_cast<int>(_sql.size()));

  if (err) {
    return make_error(_conn);
  }

  const auto raw_ptr = mysql_store_result(_conn.get());

  return Ref<MYSQL_RES>::make(
      std::shared_ptr<MYSQL_RES>(raw_ptr, mysql_free_result));
}

}  // namespace sqlgen::mysql
