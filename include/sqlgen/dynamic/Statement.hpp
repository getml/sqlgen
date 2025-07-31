#ifndef SQLGEN_DYNAMIC_STATEMENT_HPP_
#define SQLGEN_DYNAMIC_STATEMENT_HPP_

#include <rfl.hpp>

#include "CreateIndex.hpp"
#include "CreateTable.hpp"
#include "CreateTableAs.hpp"
#include "DeleteFrom.hpp"
#include "Drop.hpp"
#include "Insert.hpp"
#include "SelectFrom.hpp"
#include "Update.hpp"
#include "Write.hpp"

namespace sqlgen::dynamic {

using Statement =
    rfl::TaggedUnion<"stmt", CreateIndex, CreateTable, CreateTableAs,
                     DeleteFrom, Drop, Insert, SelectFrom, Update, Write>;

}  // namespace sqlgen::dynamic

#endif
