#include <ranges>
#include <rfl.hpp>
#include <sstream>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/sqlite/Connection.hpp"
#include "sqlgen/sqlite/Iterator.hpp"

namespace sqlgen::sqlite {

std::string column_or_value_to_sql(const dynamic::ColumnOrValue& _col) noexcept;

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept;

std::string condition_to_sql(const dynamic::Condition& _cond) noexcept;

template <class ConditionType>
std::string condition_to_sql_impl(const ConditionType& _condition) noexcept;

std::string create_index_to_sql(const dynamic::CreateIndex& _stmt) noexcept;

std::string create_table_to_sql(const dynamic::CreateTable& _stmt) noexcept;

std::string delete_from_to_sql(const dynamic::DeleteFrom& _stmt) noexcept;

std::string drop_to_sql(const dynamic::Drop& _stmt) noexcept;

template <class InsertOrWrite>
std::string insert_or_write_to_sql(const InsertOrWrite& _stmt) noexcept;

std::string properties_to_sql(const dynamic::types::Properties& _p) noexcept;

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept;

std::string type_to_sql(const dynamic::Type& _type) noexcept;

std::string update_to_sql(const dynamic::Update& _stmt) noexcept;

// ----------------------------------------------------------------------------

std::string column_or_value_to_sql(
    const dynamic::ColumnOrValue& _col) noexcept {
  const auto handle_value = [](const auto& _v) -> std::string {
    using Type = std::remove_cvref_t<decltype(_v)>;
    if constexpr (std::is_same_v<Type, dynamic::String>) {
      return "'" + _v.val + "'";
    } else {
      return std::to_string(_v.val);
    }
  };

  return _col.visit([&](const auto& _c) -> std::string {
    using Type = std::remove_cvref_t<decltype(_c)>;
    if constexpr (std::is_same_v<Type, dynamic::Column>) {
      return "\"" + _c.name + "\"";
    } else {
      return _c.visit(handle_value);
    }
  });
}

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept {
  return "\"" + _col.name + "\"" + " " + type_to_sql(_col.type) +
         properties_to_sql(
             _col.type.visit([](const auto& _t) { return _t.properties; }));
}

std::string condition_to_sql(const dynamic::Condition& _cond) noexcept {
  return _cond.val.visit(
      [&](const auto& _c) { return condition_to_sql_impl(_c); });
}

template <class ConditionType>
std::string condition_to_sql_impl(const ConditionType& _condition) noexcept {
  using C = std::remove_cvref_t<ConditionType>;
  std::stringstream stream;

  if constexpr (std::is_same_v<C, dynamic::Condition::And>) {
    stream << "(" << condition_to_sql(*_condition.cond1) << ") AND ("
           << condition_to_sql(*_condition.cond2) << ")";

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Equal>) {
    stream << column_or_value_to_sql(_condition.op1) << " = "
           << column_or_value_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::GreaterEqual>) {
    stream << column_or_value_to_sql(_condition.op1)
           << " >= " << column_or_value_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::GreaterThan>) {
    stream << column_or_value_to_sql(_condition.op1) << " > "
           << column_or_value_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::NotEqual>) {
    stream << column_or_value_to_sql(_condition.op1)
           << " != " << column_or_value_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::LesserEqual>) {
    stream << column_or_value_to_sql(_condition.op1)
           << " <= " << column_or_value_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::LesserThan>) {
    stream << column_or_value_to_sql(_condition.op1) << " < "
           << column_or_value_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Or>) {
    stream << "(" << condition_to_sql(*_condition.cond1) << ") OR ("
           << condition_to_sql(*_condition.cond2) << ")";

  } else {
    static_assert(rfl::always_false_v<C>, "Not all cases where covered.");
  }

  return stream.str();
}

std::string create_index_to_sql(const dynamic::CreateIndex& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto in_quotes = [](const std::string& _str) -> std::string {
    return "\"" + _str + "\"";
  };

  std::stringstream stream;

  if (_stmt.unique) {
    stream << "CREATE UNIQUE INDEX ";
  } else {
    stream << "CREATE INDEX ";
  }

  if (_stmt.if_not_exists) {
    stream << "IF NOT EXISTS ";
  }

  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.name << "\" ";

  stream << "ON " << "\"" << _stmt.table.name << "\"";

  stream << "(";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.columns | transform(in_quotes)));
  stream << ")";

  if (_stmt.where) {
    stream << " WHERE " << condition_to_sql(*_stmt.where);
  }

  stream << ";";

  return stream.str();
}

std::string create_table_to_sql(const dynamic::CreateTable& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto col_to_sql = [&](const auto& _col) {
    return column_to_sql_definition(_col);
  };

  std::stringstream stream;
  stream << "CREATE TABLE ";

  if (_stmt.if_not_exists) {
    stream << "IF NOT EXISTS ";
  }

  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\" ";

  stream << "(";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.columns | transform(col_to_sql)));
  stream << ");";

  return stream.str();
}

std::string delete_from_to_sql(const dynamic::DeleteFrom& _stmt) noexcept {
  std::stringstream stream;

  stream << "DELETE FROM ";

  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

  if (_stmt.where) {
    stream << " WHERE " << condition_to_sql(*_stmt.where);
  }

  stream << ";";

  return stream.str();
}

std::string drop_to_sql(const dynamic::Drop& _stmt) noexcept {
  std::stringstream stream;

  stream << "DROP TABLE ";

  if (_stmt.if_exists) {
    stream << "IF EXISTS ";
  }

  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

  stream << ";";

  return stream.str();
}

template <class InsertOrWrite>
std::string insert_or_write_to_sql(const InsertOrWrite& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto in_quotes = [](const std::string& _str) -> std::string {
    return "\"" + _str + "\"";
  };

  const auto to_questionmark = [](const std::string&) -> std::string {
    return "?";
  };

  std::stringstream stream;
  stream << "INSERT INTO ";
  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

  stream << " (";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.columns | transform(in_quotes)));
  stream << ")";

  stream << " VALUES (";
  stream << internal::strings::join(
      ", ",
      internal::collect::vector(_stmt.columns | transform(to_questionmark)));
  stream << ");";

  return stream.str();
}

std::string properties_to_sql(const dynamic::types::Properties& _p) noexcept {
  return std::string(_p.primary ? " PRIMARY KEY" : "") +
         std::string(_p.nullable ? "" : " NOT NULL");
}

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto to_str = [](const auto& _col) -> std::string {
    return "\"" + _col.name + "\"";
  };

  const auto order_by_to_str = [](const auto& _w) -> std::string {
    return "\"" + _w.column.name + "\"" + (_w.desc ? " DESC" : "");
  };

  std::stringstream stream;
  stream << "SELECT ";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.columns | transform(to_str)));

  stream << " FROM ";
  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

  if (_stmt.where) {
    stream << " WHERE " << condition_to_sql(*_stmt.where);
  }

  if (_stmt.order_by) {
    stream << " ORDER BY "
           << internal::strings::join(
                  ", ", internal::collect::vector(_stmt.order_by->columns |
                                                  transform(order_by_to_str)));
  }

  if (_stmt.limit) {
    stream << " LIMIT " << _stmt.limit->val;
  }

  stream << ";";

  return stream.str();
}

std::string to_sql_impl(const dynamic::Statement& _stmt) noexcept {
  return _stmt.visit([&](const auto& _s) -> std::string {
    using S = std::remove_cvref_t<decltype(_s)>;
    if constexpr (std::is_same_v<S, dynamic::CreateIndex>) {
      return create_index_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::CreateTable>) {
      return create_table_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::DeleteFrom>) {
      return delete_from_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Drop>) {
      return drop_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Insert>) {
      return insert_or_write_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::SelectFrom>) {
      return select_from_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Update>) {
      return update_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Write>) {
      return insert_or_write_to_sql(_s);

    } else {
      static_assert(rfl::always_false_v<S>, "Unsupported type.");
    }
  });
}

std::string type_to_sql(const dynamic::Type& _type) noexcept {
  return _type.visit([](const auto _t) -> std::string {
    using T = std::remove_cvref_t<decltype(_t)>;
    if constexpr (std::is_same_v<T, dynamic::types::Boolean> ||
                  std::is_same_v<T, dynamic::types::Int8> ||
                  std::is_same_v<T, dynamic::types::Int16> ||
                  std::is_same_v<T, dynamic::types::Int32> ||
                  std::is_same_v<T, dynamic::types::Int64> ||
                  std::is_same_v<T, dynamic::types::UInt8> ||
                  std::is_same_v<T, dynamic::types::UInt16> ||
                  std::is_same_v<T, dynamic::types::UInt32> ||
                  std::is_same_v<T, dynamic::types::UInt64>) {
      return "INTEGER";
    } else if constexpr (std::is_same_v<T, dynamic::types::Float32> ||
                         std::is_same_v<T, dynamic::types::Float64>) {
      return "REAL";
    } else if constexpr (std::is_same_v<T, dynamic::types::Unknown> ||
                         std::is_same_v<T, dynamic::types::Text> ||
                         std::is_same_v<T, dynamic::types::VarChar> ||
                         std::is_same_v<T, dynamic::types::Timestamp> ||
                         std::is_same_v<T, dynamic::types::TimestampWithTZ>) {
      return "TEXT";
    } else {
      static_assert(rfl::always_false_v<T>, "Not all cases were covered.");
    }
  });
}

std::string update_to_sql(const dynamic::Update& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto to_str = [](const auto& _set) -> std::string {
    return "\"" + _set.col.name + "\" = " + column_or_value_to_sql(_set.to);
  };

  std::stringstream stream;

  stream << "UPDATE ";

  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

  stream << " SET ";

  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.sets | transform(to_str)));

  if (_stmt.where) {
    stream << " WHERE " << condition_to_sql(*_stmt.where);
  }

  stream << ";";

  return stream.str();
}

}  // namespace sqlgen::sqlite
