#include <ranges>
#include <rfl.hpp>
#include <sstream>

#include "sqlgen/dynamic/Operation.hpp"
#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/sqlite/Connection.hpp"
#include "sqlgen/sqlite/Iterator.hpp"

namespace sqlgen::sqlite {

std::string aggregation_to_sql(
    const dynamic::Aggregation& _aggregation) noexcept;

std::string column_or_value_to_sql(const dynamic::ColumnOrValue& _col) noexcept;

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept;

std::string condition_to_sql(const dynamic::Condition& _cond) noexcept;

template <class ConditionType>
std::string condition_to_sql_impl(const ConditionType& _condition) noexcept;

std::string create_index_to_sql(const dynamic::CreateIndex& _stmt) noexcept;

std::string create_table_to_sql(const dynamic::CreateTable& _stmt) noexcept;

std::string delete_from_to_sql(const dynamic::DeleteFrom& _stmt) noexcept;

std::string drop_to_sql(const dynamic::Drop& _stmt) noexcept;

std::string escape_single_quote(const std::string& _str) noexcept;

std::string field_to_str(const dynamic::SelectFrom::Field& _field) noexcept;

template <class InsertOrWrite>
std::string insert_or_write_to_sql(const InsertOrWrite& _stmt) noexcept;

std::string operation_to_sql(const dynamic::Operation& _stmt) noexcept;

std::string properties_to_sql(const dynamic::types::Properties& _p) noexcept;

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept;

std::string type_to_sql(const dynamic::Type& _type) noexcept;

std::string update_to_sql(const dynamic::Update& _stmt) noexcept;

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

std::string column_or_value_to_sql(
    const dynamic::ColumnOrValue& _col) noexcept {
  const auto handle_value = [](const auto& _v) -> std::string {
    using Type = std::remove_cvref_t<decltype(_v)>;
    if constexpr (std::is_same_v<Type, dynamic::String>) {
      return "'" + escape_single_quote(_v.val) + "'";
    } else {
      return std::to_string(_v.val);
    }
  };

  return _col.visit([&](const auto& _c) -> std::string {
    using Type = std::remove_cvref_t<decltype(_c)>;
    if constexpr (std::is_same_v<Type, dynamic::Column>) {
      return "\"" + _c.name + "\"";
    } else {
      return _c.val.visit(handle_value);
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

std::string escape_single_quote(const std::string& _str) noexcept {
  return internal::strings::replace_all(_str, "'", "''");
}

std::string field_to_str(const dynamic::SelectFrom::Field& _field) noexcept {
  std::stringstream stream;

  stream << operation_to_sql(_field.val);

  if (_field.as) {
    stream << " AS " << "\"" << *_field.as << "\"";
  }

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
             << type_to_sql(_s.target_type) << ")";

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
      stream << "("
             << internal::strings::join(
                    " || ", internal::collect::vector(
                                _s.ops | transform([](const auto& _op) {
                                  return operation_to_sql(*_op);
                                })))
             << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Cos>) {
      stream << "cos(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Divides>) {
      stream << "(" << operation_to_sql(*_s.op1) << ") / ("
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Exp>) {
      stream << "exp(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Floor>) {
      stream << "floor(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Length>) {
      stream << "length(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Ln>) {
      stream << "ln(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Log2>) {
      stream << "log2(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Lower>) {
      stream << "lower(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::LTrim>) {
      stream << "ltrim(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Minus>) {
      stream << "(" << operation_to_sql(*_s.op1) << ") - ("
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Mod>) {
      stream << "mod(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ")";

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
      stream << "rtrim(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Sin>) {
      stream << "sin(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Sqrt>) {
      stream << "sqrt(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Tan>) {
      stream << "tan(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Trim>) {
      stream << "trim(" << operation_to_sql(*_s.op1) << ", "
             << operation_to_sql(*_s.op2) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Upper>) {
      stream << "upper(" << operation_to_sql(*_s.op1) << ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Value>) {
      stream << column_or_value_to_sql(_s);

    } else {
      static_assert(rfl::always_false_v<Type>, "Unsupported type.");
    }
    return stream.str();
  });
}

std::string properties_to_sql(const dynamic::types::Properties& _p) noexcept {
  return std::string(_p.primary ? " PRIMARY KEY" : "") +
         std::string(_p.nullable ? "" : " NOT NULL");
}

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept {
  using namespace std::ranges::views;

  const auto order_by_to_str = [](const auto& _w) -> std::string {
    return "\"" + _w.column.name + "\"" + (_w.desc ? " DESC" : "");
  };

  std::stringstream stream;

  stream << "SELECT ";
  stream << internal::strings::join(
      ", ", internal::collect::vector(_stmt.fields | transform(field_to_str)));

  stream << " FROM ";
  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

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
