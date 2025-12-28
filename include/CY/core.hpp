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
}