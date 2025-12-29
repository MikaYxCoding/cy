/**
 * @file result.hpp
 * @author Jesús Blanco
 * @brief Result implementation.
 * @version 1.0.0
 * @date 2025-12-28
 *
 * @copyright Copyright (c) Jesús Blanco. See LICENSE for details.
 *
 */

#pragma once

#include <algorithm>

namespace cy::basic_result {
namespace detail {
    template<typename T>
    constexpr
        typename std::enable_if<std::is_reference<T>::value,
                                typename std::remove_reference<T>::type *>::type
        construct_ok(T _value)
    {
        return &_value;
    }

    template<typename T>
    constexpr T&& construct_ok(
        typename std::enable_if<!std::is_reference<T>::value, T>::type& _value)
    {
        return std::move(_value);
    }
} // namespace detail

template<typename _Ty>
class Ok
{
  public:
    template<typename _Type>
    using _IsRef = std::is_reference<_Type>;
    using T =
        typename std::conditional<_IsRef<_Ty>::value,
                                  typename std::remove_reference<_Ty>::type *,
                                  _Ty>::type;

    constexpr Ok(_Ty value)
        : m_Value(detail::construct_ok<_Ty>(value))
    {
    }

    /**
     * @brief Gets the reference owned by `Ok` as a const reference.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr typename std::enable_if<
        _IsRef<T>::value,
        typename std::remove_reference<T>::type>::type const&
    get() const&
    {
        return *m_Value;
    }

    /**
     * @brief Gets the reference owned by `Ok`.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr
        typename std::enable_if<_IsRef<T>::value, T>::type
        get() &
    {
        return *m_Value;
    }

    /**
     * @brief Alias for `get()`. Gets the reference owned by `Ok`.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr
        typename std::enable_if<_IsRef<T>::value, T>::type
        take()
    {
        return *m_Value;
    }

    /**
     * @brief Gets a const reference to the value owned by `Ok`.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr
        typename std::enable_if<!_IsRef<T>::value, T>::type const&
        get() const&
    {
        return m_Value;
    }

    /**
     * @brief Gets a reference to the value owned by `Ok`.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr
        typename std::enable_if<!_IsRef<T>::value, T>::type&
        get() &
    {
        return m_Value;
    }

    /**
     * @brief Returns an rvalue reference to the vlaue owned by `Ok`, allowing
     * to move it out of `Ok`.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr
        typename std::enable_if<!_IsRef<T>::value, T>::type&&
        take()
    {
        return std::move(m_Value);
    }

  private:
    T m_Value;
};

template<>
class Ok<void>
{
  public:
    constexpr Ok() = default;
};
Ok() -> Ok<void>;

template<typename E>
class Err
{
    static_assert(
        !std::is_reference<E>::value,
        "Result<T, E&> and Err<E&> are invalid! Result must own the error");
    static_assert(!std::is_same<E, void>::value,
                  "Result<T, void> and Err<void> are invalid! Consider "
                  "Maybe<T> instead.");

  public:
    constexpr Err(E err)
        : m_Error(std::move(err))
    {
    }

    /**
     * @brief Gets a const reference (`E const&`) to the error owned by `Err`.
     *
     */
    [[nodiscard]] inline constexpr E const& get() const& { return m_Error; }
    /**
     * @brief Gets a reference (`E&`) to the error owned by `Err`.
     *
     */
    [[nodiscard]] inline constexpr E& get() & { return m_Error; }

    /**
     * @brief Gets an rvalue reference (`E&&`) to the error owned by `Err`,
     * allowing to move it out of `Err`.
     *
     */
    [[nodiscard]] inline constexpr E&& take() { return std::move(m_Error); }

  private:
    E m_Error;
};
} // namespace cy::basic_result