# `unite` and `unite_all`

The `unite` and `unite_all` functions allow you to combine the results of multiple `SELECT` statements into a single result set.

## `unite`

The `unite` function corresponds to the SQL `UNION` operator. It combines the result sets of two or more `SELECT` statements and removes duplicate rows.

### Example

```cpp
struct User1 {
  std::string name;
  int age;
};

struct User2 {
  std::string name;
  int age;
};

const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);

const auto united = sqlgen::unite<std::vector<User1>>(s1, s2);
```

## `unite_all`

The `unite_all` function corresponds to the SQL `UNION ALL` operator. It combines the result sets of two or more `SELECT` statements, including all duplicate rows.

### Example

```cpp
struct User1 {
  std::string name;
  int age;
};

struct User2 {
  std::string name;
  int age;
};

const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);

const auto united = sqlgen::unite_all<std::vector<User1>>(s1, s2);
```

## Nesting in `SELECT` statements

You can use the result of a `unite` or `unite_all` operation as a subquery in a `SELECT` statement.

### Example

```cpp
const auto united = sqlgen::unite<std::vector<User1>>(s1, s2);

const auto sel = sqlgen::select_from(united.as("u"), "name"_c, "age"_c);
```

## Nesting in `JOIN` statements

You can also use the result of a `unite` or `unite_all` operation as a subquery in a `JOIN` statement.

### Example

```cpp
struct Login {
  int id;
  std::string username;
};

const auto united = sqlgen::unite<std::vector<User1>>(s1, s2);

const auto sel =
    sqlgen::select_from<Login>(
        "id"_c, "username"_c,
        sqlgen::inner_join(united.as("u"), "username"_c == "u.name"_c))
        .where("id"_c == 1);
```
