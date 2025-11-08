#ifndef SQLGEN_PARSING_PARSER_BASE_HPP_
#define SQLGEN_PARSING_PARSER_BASE_HPP_

#include "RawType.hpp"

namespace sqlgen::parsing {

template <class T, RawType _raw_type = RawType::string>
struct Parser;

}

#endif
