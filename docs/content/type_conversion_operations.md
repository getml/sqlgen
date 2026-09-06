# Type Conversion

Casts a value to a different type (e.g., int to double).

## Motivating Example
Suppose you want to store birthdays as strings in your database, but you need to convert them to date types for calculations, or you want to cast an integer column to double for mathematical operations:

```cpp
struct BirthdayInfo {
  Date birthday;
  double age_in_days;
};

const auto get_birthdays =
    select_from<Person>(
        cast<Date>("birthday"_c).as<"birthday">(),
        days_between("birthday"_c, Date("2011-01-01")).as<"age_in_days">()
    )
    | to<std::vector<BirthdayInfo>>;
```

This query casts the `birthday` column to a `Date` type and calculates the number of days between each birthday and a reference date.

```cpp
cast<double>("age"_c) | as<"age_as_double">
