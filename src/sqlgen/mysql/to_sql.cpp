#include "sqlgen/mysql/to_sql.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "sqlgen/dynamic/Join.hpp"
#include "sqlgen/dynamic/Operation.hpp"
#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"

namespace sqlgen::mysql {

std::string aggregation_to_sql(
    const dynamic::Aggregation& _aggregation) noexcept;

std::string cast_type_to_sql(const dynamic::Type& _type) noexcept;

std::string column_or_value_to_sql(const dynamic::ColumnOrValue& _col) noexcept;

std::string condition_to_sql(const dynamic::Condition& _cond) noexcept;

template <class ConditionType>
std::string condition_to_sql_impl(const ConditionType& _condition) noexcept;

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept;

std::string create_index_to_sql(const dynamic::CreateIndex& _stmt) noexcept;

std::string create_table_to_sql(const dynamic::CreateTable& _stmt) noexcept;

std::string date_plus_duration_to_sql(
    const dynamic::Operation::DatePlusDuration& _stmt,
    const size_t _ix = 0) noexcept;

std::string delete_from_to_sql(const dynamic::DeleteFrom& _stmt) noexcept;

std::string drop_to_sql(const dynamic::Drop& _stmt) noexcept;

std::string escape_single_quote(const std::string& _str) noexcept;

std::string field_to_str(const dynamic::SelectFrom::Field& _field) noexcept;

std::string foreign_keys_to_sql(
    const std::vector<
        std::pair<std::string, dynamic::types::ForeignKeyReference>>&
        _foreign_keys) noexcept;

std::vector<std::pair<std::string, dynamic::types::ForeignKeyReference>>
get_foreign_keys(const dynamic::CreateTable& _stmt) noexcept;

std::vector<std::string> get_primary_keys(
    const dynamic::CreateTable& _stmt) noexcept;

template <class InsertOrWrite>
std::string insert_or_write_to_sql(const InsertOrWrite& _stmt) noexcept;

std::string join_to_sql(const dynamic::Join& _stmt) noexcept;

std::string operation_to_sql(const dynamic::Operation& _stmt) noexcept;

std::string properties_to_sql(const dynamic::types::Properties& _p) noexcept;

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept;

std::string table_or_query_to_sql(
    const dynamic::SelectFrom::TableOrQueryType& _table_or_query) noexcept;

std::string type_to_sql(const dynamic::Type& _type) noexcept;

std::string update_to_sql(const dynamic::Update& _stmt) noexcept;

// ----------------------------------------------------------------------------

inline std::string get_name(const dynamic::Column& _col) { return _col.name; }

inline std::string wrap_in_quotes(const std::string& _name) noexcept {
  return "`" + _name + "`";
}

// ----------------------------------------------------------------------------

std::string aggregation_to_sql(
    const dynamic::Aggregation& _aggregation) noexcept {
  return _aggregation.val.visit([](const auto& _agg) -> std::string {
    using Type = std::remove_cvref_t<decltype(_agg)>;
    std::stringstream stream;
    if constexpr (std::is_same_v<Type, dynamic::Aggregation::Avg>) {
      stream << "AVG(" << operation_to_sql(*_agg.val) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Count>) {
      const auto val =
          std::string(_agg.val && _agg.distinct ? "DISTINCT " : "") +
          (_agg.val ? column_or_value_to_sql(*_agg.val) : std::string("*"));
      stream << "COUNT(" << val << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Max>) {
      stream << "MAX(" << operation_to_sql(*_agg.val) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Min>) {
      stream << "MIN(" << operation_to_sql(*_agg.val) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Sum>) {
      stream << "SUM(" << operation_to_sql(*_agg.val) << ")";

    } else {
      static_assert(rfl::always_false_v<Type>, "Not all cases were covered.");
    }
    return stream.str();
  });
}

std::string cast_type_to_sql(const dynamic::Type& _type) noexcept {
  return _type.visit([](const auto _t) -> std::string {
    using T = std::remove_cvref_t<decltype(_t)>;
    if constexpr (std::is_same_v<T, dynamic::types::Boolean>) {
      return "BOOLEAN";

    } else if constexpr (std::is_same_v<T, dynamic::types::Int8> ||
                         std::is_same_v<T, dynamic::types::Int16> ||
                         std::is_same_v<T, dynamic::types::Int32> ||
                         std::is_same_v<T, dynamic::types::Int64>) {
      return "SIGNED";

    } else if constexpr (std::is_same_v<T, dynamic::types::UInt8> ||
                         std::is_same_v<T, dynamic::types::UInt16> ||
                         std::is_same_v<T, dynamic::types::UInt32> ||
                         std::is_same_v<T, dynamic::types::UInt64>) {
      return "UNSIGNED";

    } else if constexpr (std::is_same_v<T, dynamic::types::Float32> ||
                         std::is_same_v<T, dynamic::types::Float64>) {
      return "DECIMAL";

    } else if constexpr (std::is_same_v<T, dynamic::types::Text> ||
                         std::is_same_v<T, dynamic::types::VarChar>) {
      return "CHAR";

    } else if constexpr (std::is_same_v<T, dynamic::types::Date>) {
      return "DATE";

    } else if constexpr (std::is_same_v<T, dynamic::types::Timestamp> ||
                         std::is_same_v<T, dynamic::types::TimestampWithTZ>) {
      return "DATETIME";

    } else if constexpr (std::is_same_v<T, dynamic::types::Unknown>) {
      return "CHAR";

    } else {
      static_assert(rfl::always_false_v<T>, "Not all cases were covered.");
    }
  });
}

std::string column_or_value_to_sql(
    const dynamic::ColumnOrValue& _col) noexcept {
  const auto handle_value = [](const auto& _v) -> std::string {
    using Type = std::remove_cvref_t<decltype(_v)>;
    if constexpr (std::is_same_v<Type, dynamic::String>) {
      return "'" + escape_single_quote(_v.val) + "'";

    } else if constexpr (std::is_same_v<Type, dynamic::Duration>) {
      const auto unit =
          _v.unit == dynamic::TimeUnit::milliseconds
              ? std::string("* 1000 microsecond")
              : internal::strings::rtrim(rfl::enum_to_string(_v.unit), "s");
      return "INTERVAL " + std::to_string(_v.val) + " " + unit;

    } else if constexpr (std::is_same_v<Type, dynamic::Timestamp>) {
      return "to_timestamp(" + std::to_string(_v.seconds_since_unix) + ")";

    } else {
      return std::to_string(_v.val);
    }
  };

  return _col.visit([&](const auto& _c) -> std::string {
    using Type = std::remove_cvref_t<decltype(_c)>;
    if constexpr (std::is_same_v<Type, dynamic::Column>) {
      if (_c.alias) {
        return *_c.alias + "." + wrap_in_quotes(_c.name);
      } else {
        return wrap_in_quotes(_c.name);
      }
    } else {
      return _c.val.visit(handle_value);
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
    stream << operation_to_sql(_condition.op1) << " = "
           << operation_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::GreaterEqual>) {
    stream << operation_to_sql(_condition.op1)
           << " >= " << operation_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::GreaterThan>) {
    stream << operation_to_sql(_condition.op1) << " > "
           << operation_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::IsNull>) {
    stream << operation_to_sql(_condition.op) << " IS NULL";

  } else if constexpr (std::is_same_v<C, dynamic::Condition::IsNotNull>) {
    stream << operation_to_sql(_condition.op) << " IS NOT NULL";

  } else if constexpr (std::is_same_v<C, dynamic::Condition::LesserEqual>) {
    stream << operation_to_sql(_condition.op1)
           << " <= " << operation_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::LesserThan>) {
    stream << operation_to_sql(_condition.op1) << " < "
           << operation_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Like>) {
    stream << operation_to_sql(_condition.op) << " LIKE "
           << column_or_value_to_sql(_condition.pattern);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Not>) {
    stream << "NOT (" << condition_to_sql(*_condition.cond) << ")";

  } else if constexpr (std::is_same_v<C, dynamic::Condition::NotEqual>) {
    stream << operation_to_sql(_condition.op1)
           << " != " << operation_to_sql(_condition.op2);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::NotLike>) {
    stream << operation_to_sql(_condition.op) << " NOT LIKE "
           << column_or_value_to_sql(_condition.pattern);

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Or>) {
    stream << "(" << condition_to_sql(*_condition.cond1) << ") OR ("
           << condition_to_sql(*_condition.cond2) << ")";

  } else {
    static_assert(rfl::always_false_v<C>, "Not all cases were covered.");
  }
  return stream.str();
}

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept {
  return wrap_in_quotes(_col.name) + " " + type_to_sql(_col.type) +
         properties_to_sql(
             _col.type.visit([](const auto& _t) { return _t.properties; }));
}

std::string create_index_to_sql(const dynamic::CreateIndex& _stmt) noexcept {
  using namespace std::ranges::views;

  std::stringstream stream;

  if (_stmt.unique) {
    stream << "CREATE UNIQUE INDEX ";
  } else {
    stream << "CREATE INDEX ";
  }

  if (_stmt.if_not_exists) {
    stream << "IF NOT EXISTS ";
  }

  stream << wrap_in_quotes(_stmt.name) << " ";

  stream << "ON ";

  if (_stmt.table.schema) {
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

  stream << "(";
  stream << internal::strings::join(
      ", ",
      internal::collect::vector(_stmt.columns | transform(wrap_in_quotes)));
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

  const auto foreign_keys = get_foreign_keys(_stmt);

  if (foreign_keys.size() != 0) {
    stream << ", " << foreign_keys_to_sql(foreign_keys);
  }

  stream << ");";

  return stream.str();
}

std::string date_plus_duration_to_sql(
    const dynamic::Operation::DatePlusDuration& _stmt,
    const size_t _ix) noexcept {
  using namespace std::ranges::views;
  std::stringstream stream;
  stream << internal::strings::join(
                "",
                internal::collect::vector(
                    _stmt.durations | transform([](const auto&) -> std::string {
                      return "date_add(";
                    })))
         << operation_to_sql(*_stmt.date) << ", "
         << internal::strings::join(
                "), ", internal::collect::vector(
                           _stmt.durations | transform([](const auto& _d) {
                             return column_or_value_to_sql(dynamic::Value{_d});
                           })))
         << ")";
  return stream.str();
}

std::string delete_from_to_sql(const dynamic::DeleteFrom& _stmt) noexcept {
  std::stringstream stream;

  stream << "DELETE FROM ";

  if (_stmt.table.schema) {
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

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
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

  if (_stmt.cascade) {
    stream << " CASCADE";
  }

  stream << ";";

  return stream.str();
}

std::string escape_single_quote(const std::string& _str) noexcept {
  return internal::strings::replace_all(_str, "'", "''");
}

std::string field_to_str(const dynamic::SelectFrom::Field& _field) noexcept {
  std::stringstream stream;

  stream << operation_to_sql(_field.val);

  if (_field.as) {
    stream << " AS " << wrap_in_quotes(*_field.as);
  }

  return stream.str();
}

std::string foreign_keys_to_sql(
    const std::vector<
        std::pair<std::string, dynamic::types::ForeignKeyReference>>&
        _foreign_keys) noexcept {
  using namespace std::ranges::views;

  const auto to_str =
      [](const std::pair<std::string, dynamic::types::ForeignKeyReference>&
             _p) {
        return "FOREIGN KEY (" + wrap_in_quotes(_p.first) + ") REFERENCES " +
               wrap_in_quotes(_p.second.table) + "(" +
               wrap_in_quotes(_p.second.column) + ")";
      };

  return internal::strings::join(
      ", ", internal::collect::vector(_foreign_keys | transform(to_str)));
}

std::vector<std::pair<std::string, dynamic::types::ForeignKeyReference>>
get_foreign_keys(const dynamic::CreateTable& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto get_foreign_key_ref = [](const auto& _col)
      -> std::optional<dynamic::types::ForeignKeyReference> {
    return _col.type.visit(
        [](const auto& _t) { return _t.properties.foreign_key_reference; });
  };

  const auto has_reference = [&](const auto& _col) -> bool {
    return (true && get_foreign_key_ref(_col));
  };

  const auto to_pair = [&](const auto& _col)
      -> std::pair<std::string, dynamic::types::ForeignKeyReference> {
    return std::make_pair(get_name(_col), get_foreign_key_ref(_col).value());
  };

  return internal::collect::vector(_stmt.columns | filter(has_reference) |
                                   transform(to_pair));
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

template <class InsertOrWrite>
std::string insert_or_write_to_sql(const InsertOrWrite& _stmt) noexcept {
  using namespace std::ranges::views;

  std::stringstream stream;

  stream << "INSERT INTO ";
  if (_stmt.table.schema) {
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

  stream << " (";
  stream << internal::strings::join(
      ", ",
      internal::collect::vector(_stmt.columns | transform(wrap_in_quotes)));
  stream << ")";

  stream << " VALUES (";
  stream << internal::strings::join(
      ", ", internal::collect::vector(
                _stmt.columns |
                transform([](const auto&) -> std::string { return "?"; })));
  stream << ");";

  return stream.str();
}

std::string join_to_sql(const dynamic::Join& _stmt) noexcept {
  std::stringstream stream;

  stream << internal::strings::to_upper(internal::strings::replace_all(
                rfl::enum_to_string(_stmt.how), "_", " "))
         << " ";

  stream << table_or_query_to_sql(_stmt.table_or_query) << " ";

  stream << _stmt.alias << " ";

  if (_stmt.on) {
    stream << "ON " << condition_to_sql(*_stmt.on);
  } else {
    stream << "ON 1 = 1";
  }

  return stream.str();
}

std::string operation_to_sql(const dynamic::Operation& _stmt) noexcept {
  using namespace std::ranges::views;
  return _stmt.val.visit([](const auto& _s) -> std::string {
    using Type = std::remove_cvref_t<decltype(_s)>;

    std::stringstream stream;

    if constexpr (std::is_same_v<Type, dynamic::Operation::Abs>) {
      stream << "abs(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation>) {
      stream << aggregation_to_sql(_s);

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Cast>) {
      stream << "cast(" << operation_to_sql(*_s.op1) << " as "
             << cast_type_to_sql(_s.target_type) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Coalesce>) {
      stream << "coalesce("
             << internal::strings::join(
                    ", ", internal::collect::vector(
                              _s.ops | transform([](const auto& _op) {
                                return operation_to_sql(*_op);
                              })))
             << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Ceil>) {
      stream << "ceil(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Column>) {
      stream << column_or_value_to_sql(_s);

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Concat>) {
      stream << "concat("
             << internal::strings::join(
                    ", ", internal::collect::vector(
                              _s.ops | transform([](const auto& _op) {
                                return operation_to_sql(*_op);
                              })))
             << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Cos>) {
      stream << "cos(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type,
                                        dynamic::Operation::DatePlusDuration>) {
      return date_plus_duration_to_sql(_s);

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Day>) {
      stream << "extract(DAY from " << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type,
                                        dynamic::Operation::DaysBetween>) {
      stream << "datediff(" << operation_to_sql(*_s.op2) << ", "
             << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Divides>) {
      stream << "(" << operation_to_sql(*_s.op1) << ") / ("
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Exp>) {
      stream << "exp(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Floor>) {
      stream << "floor(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Hour>) {
      stream << "extract(HOUR from " << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Length>) {
      stream << "length(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Ln>) {
      stream << "ln(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Log2>) {
      stream << "log2( " << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Lower>) {
      stream << "lower(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::LTrim>) {
      stream << "trim(leading " << operation_to_sql(*_s.op2) << " FROM "
             << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Minus>) {
      stream << "(" << operation_to_sql(*_s.op1) << ") - ("
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Minute>) {
      stream << "extract(MINUTE from " << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Mod>) {
      stream << "mod(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Month>) {
      stream << "extract(MONTH from " << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Multiplies>) {
      stream << "(" << operation_to_sql(*_s.op1) << ") * ("
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Plus>) {
      stream << "(" << operation_to_sql(*_s.op1) << ") + ("
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Replace>) {
      stream << "replace(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ", " << operation_to_sql(*_s.op3)
             << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Round>) {
      stream << "round(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::RTrim>) {
      stream << "trim(trailing " << operation_to_sql(*_s.op2) << " FROM "
             << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Second>) {
      stream << "extract(SECOND from " << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Sin>) {
      stream << "sin(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Sqrt>) {
      stream << "sqrt(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Tan>) {
      stream << "tan(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Trim>) {
      stream << "trim(both " << operation_to_sql(*_s.op2) << " FROM "
             << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Unixepoch>) {
      stream << "unix_timestamp(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Upper>) {
      stream << "upper(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Value>) {
      stream << column_or_value_to_sql(_s);

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Weekday>) {
      stream << "dayofweek(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Year>) {
      stream << "extract(YEAR from " << operation_to_sql(*_s.op1) << ")";

    } else {
      static_assert(rfl::always_false_v<Type>, "Unsupported type.");
    }
    return stream.str();
  });
}

std::string properties_to_sql(const dynamic::types::Properties& _p) noexcept {
  return [&]() -> std::string {
    if (_p.auto_incr) {
      return " AUTO_INCREMENT";
    } else if (!_p.nullable) {
      return " NOT NULL";
    } else {
      return "";
    }
  }() + [&]() -> std::string { return _p.unique ? " UNIQUE" : ""; }();
}

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto order_by_to_str = [](const auto& _w) -> std::string {
    return column_or_value_to_sql(_w.column) + (_w.desc ? " DESC" : "");
  };

  std::stringstream stream;

  stream << "SELECT ";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.fields | transform(field_to_str)));

  stream << " FROM " << table_or_query_to_sql(_stmt.table_or_query);

  if (_stmt.alias) {
    stream << " " << *_stmt.alias;
  }

  if (_stmt.joins) {
    stream << " "
           << internal::strings::join(
                  " ", internal::collect::vector(*_stmt.joins |
                                                 transform(join_to_sql)));
  }

  if (_stmt.where) {
    stream << " WHERE " << condition_to_sql(*_stmt.where);
  }

  if (_stmt.group_by) {
    stream << " GROUP BY "
           << internal::strings::join(
                  ", ",
                  internal::collect::vector(_stmt.group_by->columns |
                                            transform(column_or_value_to_sql)));
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

  return stream.str();
}

std::string table_or_query_to_sql(
    const dynamic::SelectFrom::TableOrQueryType& _table_or_query) noexcept {
  return _table_or_query.visit([](const auto& _t) -> std::string {
    using Type = std::remove_cvref_t<decltype(_t)>;
    if constexpr (std::is_same_v<Type, dynamic::Table>) {
      return wrap_in_quotes(_t.name);
    } else {
      return "(" + select_from_to_sql(*_t) + ")";
    }
  });
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

    } else if constexpr (std::is_same_v<S, dynamic::Insert> ||
                         std::is_same_v<S, dynamic::Write>) {
      return insert_or_write_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::SelectFrom>) {
      return select_from_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Update>) {
      return update_to_sql(_s);

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

    } else if constexpr (std::is_same_v<T, dynamic::types::Int8>) {
      return "TINYINT";

    } else if constexpr (std::is_same_v<T, dynamic::types::UInt8> ||
                         std::is_same_v<T, dynamic::types::Int16>) {
      return "SMALLINT";

    } else if constexpr (std::is_same_v<T, dynamic::types::UInt16> ||
                         std::is_same_v<T, dynamic::types::Int32>) {
      return "INT";

    } else if constexpr (std::is_same_v<T, dynamic::types::UInt32> ||
                         std::is_same_v<T, dynamic::types::Int64> ||
                         std::is_same_v<T, dynamic::types::UInt64>) {
      return "BIGINT";

    } else if constexpr (std::is_same_v<T, dynamic::types::Float32> ||
                         std::is_same_v<T, dynamic::types::Float64>) {
      return "DECIMAL";

    } else if constexpr (std::is_same_v<T, dynamic::types::Text>) {
      return "TEXT";

    } else if constexpr (std::is_same_v<T, dynamic::types::VarChar>) {
      return "VARCHAR(" + std::to_string(_t.length) + ")";

    } else if constexpr (std::is_same_v<T, dynamic::types::Date>) {
      return "DATE";

    } else if constexpr (std::is_same_v<T, dynamic::types::Timestamp> ||
                         std::is_same_v<T, dynamic::types::TimestampWithTZ>) {
      return "DATETIME";

    } else if constexpr (std::is_same_v<T, dynamic::types::Unknown>) {
      return "TEXT";

    } else {
      static_assert(rfl::always_false_v<T>, "Not all cases were covered.");
    }
  });
}

std::string update_to_sql(const dynamic::Update& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto to_str = [](const auto& _set) -> std::string {
    return wrap_in_quotes(_set.col.name) + " = " +
           column_or_value_to_sql(_set.to);
  };

  std::stringstream stream;

  stream << "UPDATE ";

  if (_stmt.table.schema) {
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

  stream << " SET ";

  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.sets | transform(to_str)));

  if (_stmt.where) {
    stream << " WHERE " << condition_to_sql(*_stmt.where);
  }

  stream << ";";

  return stream.str();
}

}  // namespace sqlgen::mysql
