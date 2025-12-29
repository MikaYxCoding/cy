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
        construct(T _value)
    {
        return &_value;
    }

    template<typename T>
    T&& construct(
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
        : m_Value(detail::construct<_Ty>(value))
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
} // namespace cy::basic_result