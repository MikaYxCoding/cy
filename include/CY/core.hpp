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

namespace cy {
CY_DEFINE_RUNTIME_ERROR(unwrap_none_error, "called .unwrap() on a None value");
CY_DEFINE_RUNTIME_ERROR(get_none_error, "called .get() on a None value");
CY_DEFINE_RUNTIME_ERROR(bad_result,
                        "Result<T> had a value but was unwrapped, aka Result "
                        "does not own it anymore.");

template<typename E>
struct error_formatter
{
  public:
    constexpr error_formatter() = default;

    constexpr uint64 code(E const &) const
    {
        static_assert(false,
                      "no specialization for the provided typename! Create an "
                      "specialization of error_formatter with your error type "
                      "to use with Result.");
        return 0;
    }

    constexpr str readable(E const &) const
    {
        static_assert(false,
                      "no specialization for the provided typename! Create an "
                      "specialization of error_formatter with your error type "
                      "to use with Result.");
        return nullptr;
    }

    constexpr str readable_code(E const &) const
    {
        static_assert(false,
                      "no specialization for the provided typename! Create an "
                      "specialization of error_formatter with your error type "
                      "to use with Result.");
        return nullptr;
    }
};

template<>
struct error_formatter<std::exception>
{
    constexpr error_formatter() = default;

    constexpr uint64 code(std::exception const &) const { return 1; }
    constexpr str readable(std::exception const &e) const { return e.what(); }
    constexpr str readable_code(std::exception const &) const
    {
        return "std::exception";
    }
};

template<>
struct error_formatter<str>
{
    constexpr error_formatter() = default;

    constexpr uint64 code(str const &) const { return 1; }
    constexpr str    readable(str const &e) const { return e; }
    constexpr str    readable_code(str const &) const { return "ERROR"; }
};
}