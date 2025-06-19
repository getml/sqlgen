#include "sqlgen/postgres/to_sql.hpp"

#include <format>
#include <ranges>
#include <rfl.hpp>
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "sqlgen/dynamic/Operation.hpp"
#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"

namespace sqlgen::postgres {

std::string add_not_null_if_necessary(
    const dynamic::types::Properties& _p) noexcept;

std::string aggregation_to_sql(
    const dynamic::Aggregation& _aggregation) noexcept;

std::string column_or_value_to_sql(const dynamic::ColumnOrValue& _col) noexcept;

std::string condition_to_sql(const dynamic::Condition& _cond) noexcept;

template <class ConditionType>
std::string condition_to_sql_impl(const ConditionType& _condition) noexcept;

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept;

std::string create_index_to_sql(const dynamic::CreateIndex& _stmt) noexcept;

std::string create_table_to_sql(const dynamic::CreateTable& _stmt) noexcept;

std::string delete_from_to_sql(const dynamic::DeleteFrom& _stmt) noexcept;

std::string drop_to_sql(const dynamic::Drop& _stmt) noexcept;

std::string escape_single_quote(const std::string& _str) noexcept;

std::string field_to_str(const dynamic::SelectFrom::Field& _field) noexcept;

std::vector<std::string> get_primary_keys(
    const dynamic::CreateTable& _stmt) noexcept;

std::string insert_to_sql(const dynamic::Insert& _stmt) noexcept;

std::string operation_to_sql(const dynamic::Operation& _stmt) noexcept;

std::string select_from_to_sql(const dynamic::SelectFrom& _stmt) noexcept;

std::string type_to_sql(const dynamic::Type& _type) noexcept;

std::string update_to_sql(const dynamic::Update& _stmt) noexcept;

std::string write_to_sql(const dynamic::Write& _stmt) noexcept;

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

std::string aggregation_to_sql(
    const dynamic::Aggregation& _aggregation) noexcept {
  return _aggregation.val.visit([](const auto& _agg) -> std::string {
    using Type = std::remove_cvref_t<decltype(_agg)>;

    if constexpr (std::is_same_v<Type, dynamic::Aggregation::Avg>) {
      return std::format("AVG({})", column_or_value_to_sql(_agg.val));

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Count>) {
      const auto val =
          std::string(_agg.val && _agg.distinct ? "DISTINCT " : "") +
          (_agg.val ? column_or_value_to_sql(*_agg.val) : std::string("*"));
      return std::format("COUNT({})", val);

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Max>) {
      return std::format("MAX({})", column_or_value_to_sql(_agg.val));

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Min>) {
      return std::format("MIN({})", column_or_value_to_sql(_agg.val));

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation::Sum>) {
      return std::format("SUM({})", column_or_value_to_sql(_agg.val));

    } else {
      static_assert(rfl::always_false_v<Type>, "Not all cases were covered.");
    }
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
      return wrap_in_quotes(_c.name);
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

  if constexpr (std::is_same_v<C, dynamic::Condition::And>) {
    return std::format("({}) AND ({})", condition_to_sql(*_condition.cond1),
                       condition_to_sql(*_condition.cond2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Equal>) {
    return std::format("{} = {}", operation_to_sql(_condition.op1),
                       operation_to_sql(_condition.op2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::GreaterEqual>) {
    return std::format("{} >= {}", operation_to_sql(_condition.op1),
                       operation_to_sql(_condition.op2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::GreaterThan>) {
    return std::format("{} > {}", operation_to_sql(_condition.op1),
                       operation_to_sql(_condition.op2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::IsNull>) {
    return std::format("{} IS NULL", operation_to_sql(_condition.op));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::IsNotNull>) {
    return std::format("{} IS NOT NULL", operation_to_sql(_condition.op));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::LesserEqual>) {
    return std::format("{} <= {}", operation_to_sql(_condition.op1),
                       operation_to_sql(_condition.op2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::LesserThan>) {
    return std::format("{} < {}", operation_to_sql(_condition.op1),
                       operation_to_sql(_condition.op2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Like>) {
    return std::format("{} LIKE {}", operation_to_sql(_condition.op),
                       column_or_value_to_sql(_condition.pattern));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::NotEqual>) {
    return std::format("{} != {}", operation_to_sql(_condition.op1),
                       operation_to_sql(_condition.op2));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::NotLike>) {
    return std::format("{} NOT LIKE {}", operation_to_sql(_condition.op),
                       column_or_value_to_sql(_condition.pattern));

  } else if constexpr (std::is_same_v<C, dynamic::Condition::Or>) {
    return std::format("({}) OR ({})", condition_to_sql(*_condition.cond1),
                       condition_to_sql(*_condition.cond2));

  } else {
    static_assert(rfl::always_false_v<C>, "Not all cases were covered.");
  }
}

std::string column_to_sql_definition(const dynamic::Column& _col) noexcept {
  return wrap_in_quotes(_col.name) + " " + type_to_sql(_col.type) +
         add_not_null_if_necessary(
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

  stream << "\"" << _stmt.name << "\" ";

  stream << "ON ";

  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

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

  stream << ");";

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

  const auto to_placeholder = [](const size_t _i) -> std::string {
    return "$" + std::to_string(_i + 1);
  };

  std::stringstream stream;
  stream << "INSERT INTO ";
  if (_stmt.table.schema) {
    stream << "\"" << *_stmt.table.schema << "\".";
  }
  stream << "\"" << _stmt.table.name << "\"";

  stream << " (";
  stream << internal::strings::join(
      ", ",
      internal::collect::vector(_stmt.columns | transform(wrap_in_quotes)));
  stream << ")";

  stream << " VALUES (";
  stream << internal::strings::join(
      ", ", internal::collect::vector(
                iota(static_cast<size_t>(0), _stmt.columns.size()) |
                transform(to_placeholder)));
  stream << ");";

  return stream.str();
}

std::string operation_to_sql(const dynamic::Operation& _stmt) noexcept {
  using namespace std::ranges::views;
  return _stmt.val.visit([](const auto& _s) -> std::string {
    using Type = std::remove_cvref_t<decltype(_s)>;
    if constexpr (std::is_same_v<Type, dynamic::Operation::Abs>) {
      return std::format("abs({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Aggregation>) {
      return aggregation_to_sql(_s);

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Cast>) {
      return std::format("CAST({} AS {})", operation_to_sql(*_s.op1),
                         type_to_sql(_s.target_type));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Ceil>) {
      return std::format("ceil({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Column>) {
      return column_or_value_to_sql(_s);

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Concat>) {
      return "(" +
             internal::strings::join(
                 " || ", internal::collect::vector(
                             _s.ops | transform([](const auto& _op) {
                               return operation_to_sql(*_op);
                             }))) +
             ")";

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Cos>) {
      return std::format("cos({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Divides>) {
      return std::format("({}) / ({})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Exp>) {
      return std::format("exp({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Floor>) {
      return std::format("floor({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Length>) {
      return std::format("length({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Ln>) {
      return std::format("ln({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Log2>) {
      return std::format("log(2.0, {})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Lower>) {
      return std::format("lower({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::LTrim>) {
      return std::format("ltrim({}, {})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Minus>) {
      return std::format("({}) - ({})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Mod>) {
      return std::format("mod({}, {})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Multiplies>) {
      return std::format("({}) * ({})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Plus>) {
      return std::format("({}) + ({})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Round>) {
      return std::format("round({}, {})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::RTrim>) {
      return std::format("rtrim({}, {})", operation_to_sql(*_s.op1),
                         operation_to_sql(*_s.op2));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Sin>) {
      return std::format("sin({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Sqrt>) {
      return std::format("sqrt({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Tan>) {
      return std::format("tan({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Operation::Upper>) {
      return std::format("upper({})", operation_to_sql(*_s.op1));

    } else if constexpr (std::is_same_v<Type, dynamic::Value>) {
      return column_or_value_to_sql(_s);

    } else {
      static_assert(rfl::always_false_v<Type>, "Unsupported type.");
    }
  });
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
    stream << wrap_in_quotes(*_stmt.table.schema) << ".";
  }
  stream << wrap_in_quotes(_stmt.table.name);

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
      return insert_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::SelectFrom>) {
      return select_from_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Update>) {
      return update_to_sql(_s);

    } else if constexpr (std::is_same_v<S, dynamic::Write>) {
      return write_to_sql(_s);

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
    } else if constexpr (std::is_same_v<T, dynamic::types::Float32> ||
                         std::is_same_v<T, dynamic::types::Float64>) {
      return "NUMERIC";
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

std::string write_to_sql(const dynamic::Write& _stmt) noexcept {
  using namespace std::ranges::views;
  const auto schema = wrap_in_quotes(_stmt.table.schema.value_or("public"));
  const auto table = wrap_in_quotes(_stmt.table.name);
  const auto colnames = internal::strings::join(
      ", ",
      internal::collect::vector(_stmt.columns | transform(wrap_in_quotes)));
  return "COPY " + schema + "." + table + "(" + colnames +
         ") FROM STDIN WITH DELIMITER '\t' NULL '\e' CSV QUOTE '\a';";
}

}  // namespace sqlgen::postgres
