/**
 * @file core.hpp
 * @author Jesús Blanco
 * @brief Core definitions for CY
 * @version 1.0.0
 * @date 2025-12-28
 *
 * @copyright Copyright (c) 2025 Jesús Blanco
 *
 */

#pragma once

#include "CY/types.hpp"
#include <stdexcept>

#define CY_DEFINE_RUNTIME_ERROR(symbol, message)                               \
    class symbol : std::runtime_error                                          \
    {                                                                          \
      public:                                                                  \
        symbol()                                                               \
            : std::runtime_error(message)                                      \
        {                                                                      \
        }                                                                      \
    }

#define CY_DEFINE_READABLE_RUNTIME_ERROR(symbol, message)                      \
    class symbol : std::runtime_error                                          \
    {                                                                          \
      public:                                                                  \
        symbol(str readableErr)                                                \
            : std::runtime_error(std::string(message) + readableErr)           \
        {                                                                      \
        }                                                                      \
    }

namespace cy {
CY_DEFINE_RUNTIME_ERROR(unwrap_none_error, "called .unwrap() on a None value");
CY_DEFINE_RUNTIME_ERROR(get_none_error, "called .get() on a None value");
CY_DEFINE_RUNTIME_ERROR(bad_result,
                        "Result<T> had a value but was unwrapped, aka Result "
                        "does not own it anymore.");
CY_DEFINE_RUNTIME_ERROR(unwrap_on_ok_error,
                        "called .unwrap_err() on a Ok value");
CY_DEFINE_RUNTIME_ERROR(get_on_ok_error, "called .get_err() on a Ok value");

CY_DEFINE_READABLE_RUNTIME_ERROR(unwrap_on_err_error,
                                 "called .unwrap() on an Err value: ");
CY_DEFINE_READABLE_RUNTIME_ERROR(get_on_err_error,
                                 "called .get() on an Err value: ");

template<typename E>
struct error_formatter
{
    static_assert(false,
                  "no specialization for the provided typename! Create an "
                  "specialization of error_formatter with your error type "
                  "to use with Result.");

  public:
    error_formatter() = delete;

    static constexpr str readable(E const &) { return nullptr; }
};

template<>
struct error_formatter<str>
{
  public:
    error_formatter() = delete;

    static constexpr auto readable(str e) { return e; }
};
}