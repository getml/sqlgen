#ifndef SQLGEN_MYSQL_CREDENTIALS_HPP_
#define SQLGEN_MYSQL_CREDENTIALS_HPP_

#include <string>

namespace sqlgen::mysql {

struct Credentials {
  std::string host;
  std::string user;
  std::string password;
  std::string dbname;
  int port;
  std::string unix_socket;
};

}  // namespace sqlgen::mysql

#endif
