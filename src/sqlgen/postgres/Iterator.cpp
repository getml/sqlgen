#include "sqlgen/postgres/Iterator.hpp"

#include <ranges>
#include <rfl.hpp>
#include <sstream>

#include "sqlgen/internal/collect/vector.hpp"
#include "sqlgen/internal/strings/strings.hpp"
#include "sqlgen/postgres/exec.hpp"

namespace sqlgen::postgres {

Iterator::Iterator(const std::string& _sql, const ConnPtr& _conn)
    : cursor_name_(make_cursor_name()), conn_(_conn), end_(false) {
  exec(conn_, "BEGIN").value();
  exec(conn_, "DECLARE " + cursor_name_ + " CURSOR FOR " + _sql).value();
}

Iterator::Iterator(Iterator&& _other) noexcept
    : cursor_name_(std::move(_other.cursor_name_)),
      conn_(std::move(_other.conn_)),
      end_(_other.end_) {
  _other.end_ = true;
}

Iterator::~Iterator() { shutdown(); }

bool Iterator::end() const { return end_; }

Result<std::vector<std::vector<std::optional<std::string>>>> Iterator::next(
    const size_t _batch_size) {
  if (end()) {
    return error("End is reached.");
  }

  const auto to_vector = [](const Ref<PGresult>& _res)
      -> std::vector<std::vector<std::optional<std::string>>> {
    const int num_rows = PQntuples(_res.get());
    const int num_cols = PQnfields(_res.get());

    std::vector<std::vector<std::optional<std::string>>> vec(num_rows);

    for (int i = 0; i < num_rows; ++i) {
      std::vector<std::optional<std::string>> row(num_cols);

      for (int j = 0; j < num_cols; ++j) {
        const bool is_null = PQgetisnull(_res.get(), i, j);
        if (is_null) {
          row[j] = std::nullopt;
        } else {
          row[j] = std::string(PQgetvalue(_res.get(), i, j));
        }
      }

      vec[i] = std::move(row);
    }

    return vec;
  };

  const auto shutdown_if_necessary =
      [this](std::vector<std::vector<std::optional<std::string>>>&& _vec)
      -> std::vector<std::vector<std::optional<std::string>>> {
    if (_vec.size() == 0) {
      shutdown();
    }
    return std::move(_vec);
  };

  return exec(conn_, "FETCH FORWARD " + std::to_string(_batch_size) + " FROM " +
                         cursor_name_ + ";")
      .transform(to_vector)
      .transform(shutdown_if_necessary);
}

Iterator& Iterator::operator=(Iterator&& _other) noexcept {
  if (this == &_other) {
    return *this;
  }
  shutdown();
  cursor_name_ = std::move(_other.cursor_name_);
  conn_ = std::move(_other.conn_);
  end_ = _other.end_;
  _other.end_ = true;
  return *this;
}

void Iterator::shutdown() {
  if (!end_) {
    exec(conn_, "CLOSE " + cursor_name_);
    exec(conn_, "END");
    end_ = true;
  }
}

}  // namespace sqlgen::postgres
