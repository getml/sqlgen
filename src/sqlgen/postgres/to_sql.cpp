#include "sqlgen/postgres/to_sql.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"

namespace sqlgen::postgres {

std::string add_not_null_if_necessary(
    const dynamic::types::Properties& _p) noexcept;

std::string column_or_value_to_sql(const dynamic::ColumnOrValue& _col) noexcept;

std::string condition_to_sql(const dynamic::Condition& _cond) noexcept;

template <class ConditionType>
std::string condition_to_sql_impl(const ConditionType& _condition) noexcept;

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept;

std::string create_table_to_sql(const dynamic::CreateTable& _stmt) noexcept;

std::vector<std::string> get_primary_keys(
    const dynamic::CreateTable& _stmt) noexcept;

std::string insert_to_sql(const dynamic::Insert& _stmt) noexcept;

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept;

std::string type_to_sql(const dynamic::Type& _type) noexcept;

// ----------------------------------------------------------------------------

inline std::string get_name(const dynamic::Column& _col) { return _col.name; }

inline std::string wrap_in_quotes(const std::string& _name) noexcept {
  return "\"" + _name + "\"";
}

// ----------------------------------------------------------------------------

std::string add_not_null_if_necessary(
    const dynamic::types::Properties& _p) noexcept {
  return std::string(_p.nullable ? "" : " NOT NULL");
}

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
      return wrap_in_quotes(_c.name);
    } else {
      return _c.visit(handle_value);
    }
  });
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

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept {
  return wrap_in_quotes(_col.name) + " " + type_to_sql(_col.type) +
         add_not_null_if_necessary(
             _col.type.visit([](const auto& _t) { return _t.properties; }));
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
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name) << " ";

  stream << "(";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.columns | transform(col_to_sql)));

  const auto primary_keys = get_primary_keys(_stmt);

  if (primary_keys.size() != 0) {
    stream << ", PRIMARY KEY (" << internal::strings::join(", ", primary_keys)
           << ")";
  }

  stream << ");";

  return stream.str();
}

std::vector<std::string> get_primary_keys(
    const dynamic::CreateTable& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto is_primary_key = [](const auto& _col) -> bool {
    return _col.type.visit(
        [](const auto& _t) -> bool { return _t.properties.primary; });
  };

  return internal::collect::vector(_stmt.columns | filter(is_primary_key) |
                                   transform(get_name) |
                                   transform(wrap_in_quotes));
}

std::string insert_to_sql(const dynamic::Insert& _stmt) noexcept {
  using namespace std::ranges::views;
  const auto schema = wrap_in_quotes(_stmt.table.schema.value_or("public"));
  const auto table = wrap_in_quotes(_stmt.table.name);
  const auto colnames = internal::strings::join(
      ", ",
      internal::collect::vector(_stmt.columns | transform(wrap_in_quotes)));
  return "COPY " + schema + "." + table + "(" + colnames +
         ") FROM STDIN WITH DELIMITER '\t' NULL '\e' QUOTE '\a';";
}

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto order_by_to_str = [](const auto& _w) -> std::string {
    return "\"" + _w.column.name + "\"" + (_w.desc ? " DESC" : "");
  };

  std::stringstream stream;

  stream << "SELECT ";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.columns | transform(get_name) |
                                      transform(wrap_in_quotes)));
  stream << " FROM ";
  if (_stmt.table.schema) {
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

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
    if constexpr (std::is_same_v<S, dynamic::CreateTable>) {
      return create_table_to_sql(_s);
    } else if constexpr (std::is_same_v<S, dynamic::Insert>) {
      return insert_to_sql(_s);
    } else if constexpr (std::is_same_v<S, dynamic::SelectFrom>) {
      return select_from_to_sql(_s);
    } else {
      static_assert(rfl::always_false_v<S>, "Unsupported type.");
    }
  });
}

std::string type_to_sql(const dynamic::Type& _type) noexcept {
  return _type.visit([](const auto _t) -> std::string {
    using T = std::remove_cvref_t<decltype(_t)>;
    if constexpr (std::is_same_v<T, dynamic::types::Boolean>) {
      return "BOOLEAN";
    } else if constexpr (std::is_same_v<T, dynamic::types::Int8> ||
                         std::is_same_v<T, dynamic::types::Int16> ||
                         std::is_same_v<T, dynamic::types::UInt8> ||
                         std::is_same_v<T, dynamic::types::UInt16>) {
      return "SMALLINT";
    } else if constexpr (std::is_same_v<T, dynamic::types::Int32> ||
                         std::is_same_v<T, dynamic::types::UInt32>) {
      return "INTEGER";
    } else if constexpr (std::is_same_v<T, dynamic::types::Int64> ||
                         std::is_same_v<T, dynamic::types::UInt64>) {
      return "BIGINT";
    } else if constexpr (std::is_same_v<T, dynamic::types::Float32>) {
      return "REAL";
    } else if constexpr (std::is_same_v<T, dynamic::types::Float64>) {
      return "DOUBLE PRECISION";
    } else if constexpr (std::is_same_v<T, dynamic::types::Text>) {
      return "TEXT";
    } else if constexpr (std::is_same_v<T, dynamic::types::VarChar>) {
      return "VARCHAR(" + std::to_string(_t.length) + ")";
    } else if constexpr (std::is_same_v<T, dynamic::types::Timestamp>) {
      return "TIMESTAMP";
    } else if constexpr (std::is_same_v<T, dynamic::types::TimestampWithTZ>) {
      return "TIMESTAMP WITH TIME ZONE";
    } else if constexpr (std::is_same_v<T, dynamic::types::Unknown>) {
      return "TEXT";
    } else {
      static_assert(rfl::always_false_v<T>, "Not all cases were covered.");
    }
  });
}

}  // namespace sqlgen::postgres
