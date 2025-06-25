#ifndef SQLGEN_DYNAMIC_TIMEUNIT_HPP_
#define SQLGEN_DYNAMIC_TIMEUNIT_HPP_

namespace sqlgen::dynamic {

enum class TimeUnit {
  microseconds,
  milliseconds,
  seconds,
  minutes,
  hours,
  days,
  weeks,
  months,
  years
};

}

#endif
