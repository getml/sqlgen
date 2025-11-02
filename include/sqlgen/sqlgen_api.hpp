#ifndef SQLGEN_SQLGEN_API_HPP_
#define SQLGEN_SQLGEN_API_HPP_

#ifdef SQLGEN_BUILD_SHARED
#ifdef _WIN32
#ifdef reflectcpp_EXPORTS
#define SQLGEN_API __declspec(dllexport)
#else
#define SQLGEN_API __declspec(dllimport)
#endif
#else
#define SQLGEN_API __attribute__((visibility("default")))
#endif
#else
#define SQLGEN_API
#endif

#endif
