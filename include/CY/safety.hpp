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

#include "CY/core.hpp"
#include "CY/result.hpp"
#include <algorithm>
#include <functional>
#include <type_traits>

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
using Result = basic_result::Result<T, str>;

template<typename T>
class Maybe;

/**
 * @brief `Some` value of type `T`.
 *
 * @ref Maybe<T>
 */
template<typename T>
class Some
{
    static_assert(!std::is_void_v<T>, "Maybe<void> is invalid.");

  private:
    T val;

  public:
    friend Maybe<T>;

    explicit constexpr Some(T value)
        : val(std::move(value))
    {
    }

    /**
     * @brief Gets a const reference to `T` (`T const&`).
     */
    inline constexpr T const& get() const& { return this->val; }
    /**
     * @brief Gets a reference to `T` (`T&`).
     */
    inline constexpr T& get() & { return this->val; }
    /**
     * @brief Gets an rvalue reference to `T` (`T&&`), allowing to move it out
     * of `Some`.
     */
    inline const T&& take() { return std::move(this->val); }
};

/**
 * @attention This is a template specialization for `T&`. Working with
 * references directly is tricky, so both `Some<T&>` and `Maybe<T&>` hold `T*`
 * instead of an actual reference.
 * @brief `Some` value of type `T&`.
 *
 * @ref Maybe<T&>
 * @ref Some<T>
 */
template<typename T>
class Some<T&>
{
    static_assert(!std::is_void_v<T>, "Maybe<void> is invalid.");

  private:
    T *val;

  public:
    friend Maybe<T&>;

    explicit constexpr Some(T& value)
        : val(&value)
    {
    }

    /**
     * @brief Gets a const reference to `T` (`T const&`).
     *
     */
    inline constexpr T const& get() const& { return *this->val; }
    /**
     * @brief Gets a reference to `T` (`T &`).
     *
     */
    inline constexpr T& get() & { return *this->val; }
};

/**
 * @brief Represents the absence of a value.
 *
 */
class None
{
  public:
    constexpr None() = default;
};

/**
 * @brief `Maybe<T>` represents a value that might or might not exist. If it's
 * `Some<T>`, then the value exists. If it is `None`, then it does not exist.
 */
template<typename T>
class Maybe
{
    static_assert(!std::is_void_v<T>, "Maybe<void> is invalid.");

  private:
    bool has_value;
    union
    {
        T value;
    };

    inline T const& get_unchecked() const& { return this->value; }
    inline T&       get_unchecked() & { return this->value; }
    inline T&&      unwrap_unchecked()
    {
        this->has_value = false;
        return std::move(this->value);
    }

  public:
    ~Maybe()
    {
        if (this->has_value)
            this->value.~T();
    }

    constexpr Maybe(Some<T> some)
        : has_value(true)
        , value(std::move(some.val))
    {
    }

    constexpr Maybe(None)
        : has_value(false)
    {
    }

    constexpr Maybe()
        : has_value(false)
    {
    }

    /**
     * @brief Whether this `Maybe<T>` is `Some<T>`.
     *
     * @return true If it is.
     * @return false If it isn't.
     */
    inline constexpr bool is_some() const { return this->has_value; }
    /**
     * @brief Opposite of `is_some`.
     */
    inline constexpr bool is_none() const { return !this->is_some(); }

    /**
     * @brief Gets a const reference to `T` (`T const&`).
     *
     * @exception cy::get_none_error Thrown if `Maybe<T>` doesn't actually have
     * a value.
     */
    constexpr T const& get() const&
    {
        if (!this->has_value)
            throw get_none_error();

        return this->get_unchecked();
    }
    /**
     * @brief Gets a reference to `T` (`T&`).
     *
     * @exception cy::get_none_error Thrown if `Maybe<T>` doesn't actually have
     * a value.
     */
    constexpr T& get() &
    {
        if (!this->has_value)
            throw get_none_error();

        return this->get_unchecked();
    }
    /**
     * @brief Unwraps the value in `Maybe<T>`, allowing to move it out.
     *
     * @exception cy::unwrap_none_error Thrown if `Maybe<T>` doesn't actually
     * have a value.
     */
    constexpr T&& unwrap()
    {
        if (!this->has_value)
            throw unwrap_none_error();

        return this->unwrap_unchecked();
    }

    /**
     * @brief Maps a `Maybe<T>` to a `Maybe<U>` by taking a function that maps
     * `T` to `U` and running it if `Maybe<T>` is `Some<T>`.
     *
     * @param func Function mapping `T` to `U`.
     */
    template<typename U>
    Maybe<U> map(std::function<U(T)> func)
    {
        if (this->has_value) {
            return Some(func(this->unwrap_unchecked()));
        }

        return None();
    }
};

/**
 * @attention This is a template specialization for `T&`. Working with
 * references directly is tricky, so both `Some<T&>` and `Maybe<T&>` hold `T*`
 * instead of an actual reference.
 * @brief `Some` value of type `T&`.
 *
 * @ref Some<T&>
 * @ref Some<T>
 */
template<typename T>
class Maybe<T&>
{
    static_assert(!std::is_void_v<T>, "Maybe<void> is invalid.");

  private:
    bool has_value;
    T   *value;

    inline T& unwrap_unchecked()
    {
        this->has_value = false;
        return *this->value;
    }

  public:
    constexpr Maybe(Some<T&> some)
        : has_value(true)
        , value(some.val)
    {
    }

    constexpr Maybe(None)
        : has_value(false)
    {
    }

    constexpr Maybe()
        : has_value(false)
    {
    }

    /**
     * @brief Whether this `Maybe<T>` is `Some<T>`.
     *
     * @return true If it is.
     * @return false If it isn't.
     */
    inline constexpr bool is_some() const { return this->has_value; }
    /**
     * @brief Opposite of `is_some`.
     */
    inline constexpr bool is_none() const { return !this->is_some(); }

    /**
     * @brief Unwraps the reference in `Maybe<T&>`, allowing to move it out.
     *
     * @exception cy::unwrap_none_error Thrown if `Maybe<T&>` doesn't actually
     * have a reference.
     */
    constexpr T& unwrap()
    {
        if (!this->has_value)
            throw unwrap_none_error();

        return this->unwrap_unchecked();
    }

    /**
     * @brief Maps a `Maybe<T&>` to a `Maybe<U>` by taking a function that maps
     * `T&` to `U` and running it if `Maybe<T&>` is `Some<T&>`.
     *
     * @param func Function mapping `T&` to `U`.
     */
    template<typename U>
    Maybe<U> map(std::function<U(T&)> func)
    {
        if (this->has_value) {
            return Some(func(this->unwrap_unchecked()));
        }

        return None();
    }
};
} // namespace cy