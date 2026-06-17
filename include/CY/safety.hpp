/**
 * @file safety.hpp
 * @author Jesús Blanco
 * @brief General code safety with value/error and value/none classes.
 * @version 3.0.0
 * @date 2025-12-28
 *
 * @copyright Copyright (c) Jesús Blanco. See LICENSE for details.
 *
 */

#pragma once

#include "CY/result.hpp"

namespace cy {
/**
 * @brief Represents an operation that might fail. It returns `Ok` on success,
 * and `Err` on failure. This is a template specialization for the most common
 * use case of `Result<T, str>` (the error type is just an string error
 * message). If you want to return some other error type, use
 * cy::basic_result::Result, and specialize cy::error_formatter for you error
 * type.
 */
template<typename T>
using Result = result::Result<T, str>;
} // namespace cy
