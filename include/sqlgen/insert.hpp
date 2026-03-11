#ifndef SQLGEN_INSERT_HPP_
#define SQLGEN_INSERT_HPP_

#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <rfl.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "dynamic/Insert.hpp"
#include "internal/has_auto_incr_primary_key.hpp"
#include "internal/has_constraint.hpp"
#include "is_connection.hpp"
#include "parsing/Parser.hpp"
#include "transpilation/to_insert_or_write.hpp"
#include "transpilation/value_t.hpp"

namespace sqlgen {

namespace conflict_policy {

struct replace {};

struct ignore {};

inline constexpr replace or_replace{};

inline constexpr ignore or_ignore{};

}  // namespace conflict_policy

using conflict_policy::or_ignore;
using conflict_policy::or_replace;

namespace internal::insert {

template <class T>
concept OutputIDRange =
    std::ranges::range<T> && requires(T& t, typename T::value_type v) {
      t.clear();
      t.push_back(v);
    };

template <OutputIDRange IDsType>
struct ReturningModifier {
  IDsType* ids;
};

enum class ModifierKind { unsupported, conflict_policy, returning };

template <class Modifier>
struct modifier_traits {
  static constexpr ModifierKind kind = ModifierKind::unsupported;
  static constexpr dynamic::Insert::ConflictPolicy conflict_policy =
      dynamic::Insert::ConflictPolicy::none;
  using ids_type = void;
};

template <>
struct modifier_traits<conflict_policy::replace> {
  static constexpr ModifierKind kind = ModifierKind::conflict_policy;
  static constexpr dynamic::Insert::ConflictPolicy conflict_policy =
      dynamic::Insert::ConflictPolicy::replace;
  using ids_type = void;
};

template <>
struct modifier_traits<conflict_policy::ignore> {
  static constexpr ModifierKind kind = ModifierKind::conflict_policy;
  static constexpr dynamic::Insert::ConflictPolicy conflict_policy =
      dynamic::Insert::ConflictPolicy::ignore;
  using ids_type = void;
};

template <OutputIDRange IDsType>
struct modifier_traits<ReturningModifier<IDsType>> {
  static constexpr ModifierKind kind = ModifierKind::returning;
  static constexpr dynamic::Insert::ConflictPolicy conflict_policy =
      dynamic::Insert::ConflictPolicy::none;
  using ids_type = IDsType;
};

template <class Modifier>
using modifier_traits_t = modifier_traits<std::remove_cvref_t<Modifier>>;

template <class Modifier>
constexpr bool is_supported_modifier_v =
    modifier_traits_t<Modifier>::kind != ModifierKind::unsupported;

template <class Modifier>
constexpr bool is_conflict_policy_modifier_v =
    modifier_traits_t<Modifier>::kind == ModifierKind::conflict_policy;

template <class Modifier>
constexpr bool is_returning_modifier_v =
    modifier_traits_t<Modifier>::kind == ModifierKind::returning;

template <class Modifier>
constexpr bool is_ignore_modifier_v =
    modifier_traits_t<Modifier>::conflict_policy ==
    dynamic::Insert::ConflictPolicy::ignore;

template <OutputIDRange IDsType>
Result<Nothing> assign_returning_ids(
    IDsType* _ids, const std::vector<std::optional<std::string>>& _raw_ids) {
  using ValueType = typename IDsType::value_type;

  _ids->clear();

  if constexpr (requires(IDsType& _v, size_t _n) { _v.reserve(_n); }) {
    _ids->reserve(_raw_ids.size());
  }

  for (const auto& raw_id : _raw_ids) {
    auto parsed = parsing::Parser<ValueType>::read(raw_id);

    if (!parsed) {
      return error("Could not parse returned id: " +
                   std::string(parsed.error().what()));
    }

    _ids->push_back(parsed.value());
  }

  return Nothing{};
}

template <class... Modifiers>
constexpr void validate_modifiers() {
  static_assert((true && ... && is_supported_modifier_v<Modifiers>),
                "Unsupported insert modifier. Supported modifiers are "
                "sqlgen::or_replace, "
                "sqlgen::or_ignore and sqlgen::returning(ids).\n"
                "Example: insert(rows, or_replace, returning(ids)).");

  constexpr auto num_conflict_policies =
      (0 + ... + (is_conflict_policy_modifier_v<Modifiers> ? 1 : 0));
  constexpr auto num_returning =
      (0 + ... + (is_returning_modifier_v<Modifiers> ? 1 : 0));
  constexpr bool has_ignore = (false || ... || is_ignore_modifier_v<Modifiers>);

  static_assert(num_conflict_policies <= 1,
                "You can only set one conflict policy on insert(...).\n"
                "Use either sqlgen::or_replace or sqlgen::or_ignore.");

  static_assert(num_returning <= 1,
                "You can only call returning(ids) once on insert(...).");

  static_assert(!(has_ignore && num_returning != 0),
                "You cannot combine returning(ids) with or_ignore.");
}

template <class... Modifiers>
constexpr dynamic::Insert::ConflictPolicy conflict_policy_from_modifiers() {
  constexpr bool has_replace =
      (false || ... ||
       (modifier_traits_t<Modifiers>::conflict_policy ==
        dynamic::Insert::ConflictPolicy::replace));
  constexpr bool has_ignore = (false || ... || is_ignore_modifier_v<Modifiers>);

  if constexpr (has_replace) {
    return dynamic::Insert::ConflictPolicy::replace;
  } else if constexpr (has_ignore) {
    return dynamic::Insert::ConflictPolicy::ignore;
  } else {
    return dynamic::Insert::ConflictPolicy::none;
  }
}

template <class... Modifiers>
struct ids_type_from_modifiers {
  using type = void;
};

template <class First, class... Rest>
struct ids_type_from_modifiers<First, Rest...> {
 private:
  using current_ids_type = typename modifier_traits_t<First>::ids_type;
  using rest_ids_type = typename ids_type_from_modifiers<Rest...>::type;

 public:
  using type = std::conditional_t<std::is_void_v<current_ids_type>,
                                  rest_ids_type, current_ids_type>;
};

template <class... Modifiers>
using ids_type_t =
    typename ids_type_from_modifiers<std::remove_cvref_t<Modifiers>...>::type;

inline std::nullptr_t extract_ids_ptr() { return nullptr; }

template <class Modifier, class... Rest>
auto extract_ids_ptr(const Modifier& _modifier, const Rest&... _rest) {
  if constexpr (is_returning_modifier_v<Modifier>) {
    return _modifier.ids;
  } else {
    return extract_ids_ptr(_rest...);
  }
}

template <dynamic::Insert::ConflictPolicy _conflict_policy, class IDsType>
struct ParsedModifiers {
  static constexpr dynamic::Insert::ConflictPolicy conflict_policy =
      _conflict_policy;
  using ids_type = IDsType;
  std::conditional_t<std::is_void_v<IDsType>, std::nullptr_t, IDsType*>
      ids_ptr = nullptr;
};

template <class... Modifiers>
auto parse_modifiers(const Modifiers&... _modifiers) {
  validate_modifiers<Modifiers...>();

  constexpr auto conflict_policy =
      conflict_policy_from_modifiers<Modifiers...>();
  using IDsType = ids_type_t<Modifiers...>;

  if constexpr (std::is_void_v<IDsType>) {
    return ParsedModifiers<conflict_policy, IDsType>{};
  } else {
    return ParsedModifiers<conflict_policy, IDsType>{
        .ids_ptr = extract_ids_ptr(_modifiers...)};
  }
}

template <class T>
struct is_connection_handle : std::false_type {};

template <class Connection>
struct is_connection_handle<Ref<Connection>>
    : std::bool_constant<is_connection<Connection>> {};

template <class Connection>
struct is_connection_handle<Result<Ref<Connection>>>
    : std::bool_constant<is_connection<Connection>> {};

template <class T>
constexpr bool is_connection_handle_v =
    is_connection_handle<std::remove_cvref_t<T>>::value;

template <class Connection, class = void>
struct returning_capabilities {
  static constexpr bool supports_returning_ids = false;
  static constexpr bool supports_multirow_returning_ids = false;
};

template <class Connection>
struct returning_capabilities<
    Connection,
    std::void_t<
        decltype(std::remove_cvref_t<Connection>::supports_returning_ids),
        decltype(std::remove_cvref_t<
                 Connection>::supports_multirow_returning_ids)>> {
  static constexpr bool supports_returning_ids =
      std::remove_cvref_t<Connection>::supports_returning_ids;
  static constexpr bool supports_multirow_returning_ids =
      std::remove_cvref_t<Connection>::supports_multirow_returning_ids;
};

template <dynamic::Insert::ConflictPolicy _conflict_policy, bool _has_returning,
          bool _single_row_hint, class TableType, class Connection>
constexpr void validate_insert_usage() {
  if constexpr (_conflict_policy == dynamic::Insert::ConflictPolicy::replace) {
    static_assert(internal::has_constraint_v<TableType>,
                  "The table must have a primary key or unique column for "
                  "insert_or_replace(...) to work.");
  }

  if constexpr (_has_returning) {
    static_assert(internal::has_auto_incr_primary_key_v<TableType>,
                  "The table must have an auto-incrementing primary key for "
                  "returning(ids) to work.");

    static_assert(_conflict_policy != dynamic::Insert::ConflictPolicy::ignore,
                  "You cannot combine returning(ids) with or_ignore.");

    static_assert(returning_capabilities<Connection>::supports_returning_ids,
                  "The current backend does not support returning(ids).");

    if constexpr (!returning_capabilities<
                      Connection>::supports_multirow_returning_ids) {
      static_assert(_single_row_hint,
                    "This backend only supports returning(ids) for single-"
                    "object inserts.");
    }
  }
}

}  // namespace internal::insert

template <class ContainerType>
  requires(!std::is_const_v<std::remove_reference_t<ContainerType>>)
auto returning(ContainerType& _ids) {
  return internal::insert::ReturningModifier<ContainerType>{.ids = &_ids};
}

template <class ContainerType>
auto returning(const std::reference_wrapper<ContainerType> _ids) {
  return returning(_ids.get());
}

template <dynamic::Insert::ConflictPolicy _conflict_policy,
          bool _single_row_hint = false, class ItBegin, class ItEnd,
          class Connection, class IDsPtr = std::nullptr_t>
  requires is_connection<Connection> &&
           std::input_or_output_iterator<ItBegin> &&
           std::sentinel_for<ItEnd, ItBegin>
Result<Ref<Connection>> insert_impl(const Ref<Connection>& _conn,
                                    ItBegin _begin, ItEnd _end,
                                    IDsPtr _ids = nullptr) {
  using T =
      std::remove_cvref_t<typename std::iterator_traits<ItBegin>::value_type>;

  constexpr bool has_returning = !std::is_same_v<IDsPtr, std::nullptr_t>;

  internal::insert::validate_insert_usage<_conflict_policy, has_returning,
                                          _single_row_hint, T, Connection>();

  const auto insert_stmt =
      transpilation::to_insert_or_write<T, dynamic::Insert>(_conflict_policy,
                                                            has_returning);

  if constexpr (has_returning) {
    std::vector<std::optional<std::string>> raw_ids;

    return _conn->insert(insert_stmt, _begin, _end, &raw_ids)
        .and_then([&](const auto&) {
          return internal::insert::assign_returning_ids(_ids, raw_ids);
        })
        .transform([&](const auto&) { return _conn; });
  }

  return _conn->insert(insert_stmt, _begin, _end).transform([&](const auto&) {
    return _conn;
  });
}

template <dynamic::Insert::ConflictPolicy _conflict_policy,
          bool _single_row_hint = false, class ItBegin, class ItEnd,
          class Connection, class IDsPtr = std::nullptr_t>
  requires is_connection<Connection> &&
           std::input_or_output_iterator<ItBegin> &&
           std::sentinel_for<ItEnd, ItBegin>
Result<Ref<Connection>> insert_impl(const Result<Ref<Connection>>& _res,
                                    ItBegin _begin, ItEnd _end,
                                    IDsPtr _ids = nullptr) {
  return _res.and_then([&](const auto& _conn) {
    return insert_impl<_conflict_policy, _single_row_hint>(_conn, _begin, _end,
                                                           _ids);
  });
}

template <dynamic::Insert::ConflictPolicy _conflict_policy, class ContainerType,
          class IDsPtr = std::nullptr_t>
auto insert_impl(const auto& _conn, const ContainerType& _data,
                 IDsPtr _ids = nullptr) {
  if constexpr (std::ranges::input_range<std::remove_cvref_t<ContainerType>>) {
    return insert_impl<_conflict_policy, false>(_conn, _data.begin(),
                                                _data.end(), _ids);
  } else {
    return insert_impl<_conflict_policy, true>(_conn, &_data, &_data + 1, _ids);
  }
}

template <dynamic::Insert::ConflictPolicy _conflict_policy, class ContainerType,
          class IDsPtr = std::nullptr_t>
auto insert_impl(const auto& _conn,
                 const std::reference_wrapper<ContainerType>& _data,
                 IDsPtr _ids = nullptr) {
  return insert_impl<_conflict_policy>(_conn, _data.get(), _ids);
}

template <class ContainerType,
          dynamic::Insert::ConflictPolicy _conflict_policy =
              dynamic::Insert::ConflictPolicy::none,
          class IDsType = void>
struct Insert {
  using ValueType = transpilation::value_t<ContainerType>;

  auto operator()(const auto& _conn) const {
    if constexpr (std::is_void_v<IDsType>) {
      return insert_impl<_conflict_policy>(_conn, data_);
    } else {
      return insert_impl<_conflict_policy>(_conn, data_, ids_);
    }
  }

  template <class Modifier>
  friend auto operator|(const Insert& _insert, const Modifier& _modifier) {
    using ModifierType = std::remove_cvref_t<Modifier>;

    static_assert(internal::insert::is_supported_modifier_v<ModifierType>,
                  "Unsupported insert modifier. Supported modifiers are "
                  "sqlgen::or_replace, "
                  "sqlgen::or_ignore and sqlgen::returning(ids).\n"
                  "Example: insert(rows, or_replace, returning(ids)).");

    if constexpr (internal::insert::is_returning_modifier_v<ModifierType>) {
      using NewIDsType =
          typename internal::insert::modifier_traits_t<ModifierType>::ids_type;

      static_assert(std::is_void_v<IDsType>,
                    "You can only call returning(ids) once on insert(...).");
      static_assert(_conflict_policy != dynamic::Insert::ConflictPolicy::ignore,
                    "You cannot combine returning(ids) with or_ignore.");
      static_assert(internal::has_auto_incr_primary_key_v<ValueType>,
                    "The table must have an auto-incrementing primary key for "
                    "returning(ids) to work.");

      return Insert<ContainerType, _conflict_policy, NewIDsType>{
          .data_ = _insert.data_, .ids_ = _modifier.ids};
    } else {
      constexpr auto next_conflict_policy =
          internal::insert::modifier_traits_t<ModifierType>::conflict_policy;

      static_assert(
          next_conflict_policy != dynamic::Insert::ConflictPolicy::none,
          "Unsupported insert modifier.");
      static_assert(_conflict_policy == dynamic::Insert::ConflictPolicy::none,
                    "You can only set one conflict policy on insert(...).\n"
                    "Use either sqlgen::or_replace or sqlgen::or_ignore.");
      static_assert(
          !(next_conflict_policy == dynamic::Insert::ConflictPolicy::ignore &&
            !std::is_void_v<IDsType>),
          "You cannot combine returning(ids) with or_ignore.");

      if constexpr (next_conflict_policy ==
                    dynamic::Insert::ConflictPolicy::replace) {
        static_assert(internal::has_constraint_v<ValueType>,
                      "The table must have a primary key or unique column for "
                      "insert_or_replace(...) to work.");
      }

      return Insert<ContainerType, next_conflict_policy, IDsType>{
          .data_ = _insert.data_, .ids_ = _insert.ids_};
    }
  }

  ContainerType data_;
  [[no_unique_address]]
  std::conditional_t<std::is_void_v<IDsType>, std::monostate, IDsType*> ids_{};
};

template <class ContainerType, class... Modifiers>
  requires(!internal::insert::is_connection_handle_v<ContainerType>)
auto insert(const ContainerType& _data, const Modifiers&... _modifiers) {
  const auto parsed_modifiers =
      internal::insert::parse_modifiers(_modifiers...);
  using ParsedModifiers = std::remove_cvref_t<decltype(parsed_modifiers)>;
  using ValueType = transpilation::value_t<ContainerType>;
  using IDsType = typename ParsedModifiers::ids_type;

  constexpr auto conflict_policy = ParsedModifiers::conflict_policy;

  if constexpr (conflict_policy == dynamic::Insert::ConflictPolicy::replace) {
    static_assert(internal::has_constraint_v<ValueType>,
                  "The table must have a primary key or unique column for "
                  "insert_or_replace(...) to work.");
  }

  if constexpr (!std::is_void_v<IDsType>) {
    static_assert(internal::has_auto_incr_primary_key_v<ValueType>,
                  "The table must have an auto-incrementing primary key for "
                  "returning(ids) to work.");

    return Insert<ContainerType, conflict_policy, IDsType>{
        .data_ = _data, .ids_ = parsed_modifiers.ids_ptr};
  } else {
    return Insert<ContainerType, conflict_policy>{.data_ = _data};
  }
}

template <class ConnectionHandle, class ContainerType, class... Modifiers>
  requires internal::insert::is_connection_handle_v<ConnectionHandle> &&
           (!std::input_or_output_iterator<std::remove_cvref_t<ContainerType>>)
auto insert(const ConnectionHandle& _conn, const ContainerType& _data,
            const Modifiers&... _modifiers) {
  return insert(_data, _modifiers...)(_conn);
}

template <class ConnectionHandle, class ItBegin, class ItEnd,
          class... Modifiers>
  requires internal::insert::is_connection_handle_v<ConnectionHandle> &&
           std::input_or_output_iterator<ItBegin> &&
           std::sentinel_for<ItEnd, ItBegin>
auto insert(const ConnectionHandle& _conn, ItBegin _begin, ItEnd _end,
            const Modifiers&... _modifiers) {
  return insert(std::ranges::subrange(_begin, _end), _modifiers...)(_conn);
}

template <class ConnectionHandle, class ContainerType>
  requires internal::insert::is_connection_handle_v<ConnectionHandle>
[[deprecated(
    "Use `insert(...) | or_replace` instead of `insert_or_replace(...)`.")]]
auto insert_or_replace(const ConnectionHandle& _conn,
                       const ContainerType& _data) {
  static_assert(
      internal::has_constraint_v<transpilation::value_t<ContainerType>>,
      "The table must have a primary key or unique column for "
      "insert_or_replace(...) to work.");

  return insert(_conn, _data, or_replace);
}

template <class ContainerType>
[[deprecated(
    "Use `insert(...) | or_replace` instead of `insert_or_replace(...)`.")]]
auto insert_or_replace(const ContainerType& _data) {
  static_assert(
      internal::has_constraint_v<transpilation::value_t<ContainerType>>,
      "The table must have a primary key or unique column for "
      "insert_or_replace(...) to work.");

  return insert(_data, or_replace);
}

}  // namespace sqlgen

#endif
