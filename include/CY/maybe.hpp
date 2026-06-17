/**
 * @file maybe.hpp
 * @author Jesús Blanco
 * @brief Maybe implementation.
 * @version 4.0.0
 * @date 2026-06-17
 *
 * @copyright Copyright (c) Jesús Blanco. See LICENSE for details.
 *
 */

#pragma once

#include "CY/core.hpp"
#include <algorithm>
#include <functional>

namespace cy {
namespace detail {
    template<typename _Ty>
    using _IsRef = std::is_reference<_Ty>;

    template<typename _Ty>
    using _PtrIfRef =
        typename std::conditional<_IsRef<_Ty>::value,
                                  typename std::remove_reference<_Ty>::type *,
                                  _Ty>::type;
} // namespace detail

/**
 * @brief `Some` value of type `T`.
 *
 */
template<typename T>
class Some
{
  public:
    T value;

    explicit constexpr Some(T _value)
        requires detail::_IsRef<T>::value
        : value(_value)
    {
    }

    explicit constexpr Some(T _value)
        : value(std::move(_value))
    {
    }
};
/**
 * @brief No value.
 *
 */
class None
{
  public:
    explicit constexpr None() = default;
};

/**
 * @brief Represents a value that might or might not exist.
 */
template<typename _Ty>
class Maybe
{
    using T = detail::_PtrIfRef<_Ty>;

  public:
    constexpr Maybe(Some<_Ty> some)
        requires detail::_IsRef<_Ty>::value
        : m_Value(&some.value)
        , m_HasValue(true)
    {
    }

    constexpr Maybe(Some<_Ty> some)
        : m_Value(std::move(some.value))
        , m_HasValue(true)
    {
    }

    constexpr Maybe(None)
        : m_HasValue(false)
    {
    }

    constexpr ~Maybe()
        requires std::is_trivially_destructible<_Ty>::value
    = default;

    constexpr ~Maybe()
    {
        if (m_HasValue)
            m_Value.~_Ty();
    }

    Maybe(Maybe const&)
        requires std::is_trivially_copyable<_Ty>::value
    = default;

    Maybe& operator=(Maybe const&)
        requires std::is_trivially_copyable<_Ty>::value
    = default;

    Maybe(Maybe const& other)
    {
        m_HasValue = other.m_HasValue;
        if (m_HasValue)
            m_Value = other.m_Value;
    }

    Maybe& operator=(Maybe const& other)
    {
        m_HasValue = other.m_HasValue;
        if (m_HasValue)
            m_Value = other.m_Value;

        return *this;
    }

    Maybe(Maybe&&)
        requires std::is_trivially_move_constructible<_Ty>::value
    = default;

    Maybe& operator=(Maybe&&)
        requires std::is_trivially_move_assignable<_Ty>::value
    = default;

    Maybe(Maybe&& source)
    {
        m_HasValue = source.m_HasValue;
        if (m_HasValue)
            m_Value = std::move(source.m_Value);
    }

    Maybe& operator=(Maybe&& source)
    {
        if (this != &source) {
            m_HasValue = source.m_HasValue;
            if (m_HasValue)
                m_Value = std::move(source.m_Value);
        }

        return *this;
    }

    /**
     * @brief Whether this `Maybe` is `Some`.
     *
     */
    constexpr bool is_some() const { return m_HasValue; }
    /**
     * @brief Whether this `Maybe` is `None`.
     *
     */
    constexpr bool is_none() const { return !m_HasValue; }

    /**
     * @brief Whether this `Maybe` is `Some` and matches a predicate `f`.
     *
     * @param f The predicate to evaluate.
     */
    inline bool is_some_and(std::function<bool(_Ty)> f) const
        requires detail::_IsRef<_Ty>::value
    {
        if (this->is_none())
            return false;

        return f(*m_Value);
    }
    /**
     * @brief Whether this `Maybe` is `Some` and matches a predicate `f`.
     *
     * @param f The predicate to evaluate.
     */
    inline bool is_some_and(std::function<bool(_Ty const&)> f) const
    {
        if (this->is_none())
            return false;

        return f(m_Value);
    }
    /**
     * @brief Returns the contained `Some` value, allowing to move it out of
     * `Maybe`. Since this function moves out the value, it should only be
     * called once.
     *
     * @exception cy::unwrap_none_error Thrown if `Maybe` does not
     * contain an `Ok` value.
     *
     */
    [[nodiscard]] inline auto&& unwrap()
    {
        if (this->is_none())
            throw cy::unwrap_none_error();

        return this->unwrap_unchecked();
    }
    /**
     * @brief Returns a const reference to the contained `Some` value. This
     * function does not move anything and it's perfectly safe to call multiple
     * times.
     *
     * @exception cy::unwrap_none_error Thrown if `Maybe` does not
     * contain an `Ok` value.
     *
     */
    [[nodiscard]] inline auto&& get() const
    {
        if (this->is_none())
            throw cy::get_none_error();

        return this->get_unchecked();
    }
    /**
     * @brief Returns a reference to the contained `Some` value. This
     * function does not move anything and it's perfectly safe to call multiple
     * times.
     *
     * @exception cy::unwrap_none_error Thrown if `Maybe` does not
     * contain an `Ok` value.
     *
     */
    [[nodiscard]] inline auto&& get()
    {
        if (this->is_none())
            throw cy::get_none_error();

        return this->get_unchecked();
    }
    /**
     * @brief Converts this `Maybe<T>` into a `Maybe<U>` by using a function
     * that accepts `T` and returns `U`. The provided function is only called if
     * `Maybe` is `Some`, otherwise nothing happens and `None` is returned.
     *
     * @param f Function mapping `T` to `U`.
     */
    template<typename U>
    [[nodiscard]] inline Maybe<U> map(std::function<U(_Ty)> f)
    {
        if (this->is_none())
            return None();

        return Some<U>(f(this->unwrap_unchecked()));
    }

    /**
     * @brief Converts this `Maybe` into a boolean. Equivalent to
     * `Maybe::is_some`.
     *
     */
    [[nodiscard]] constexpr operator bool() const { return this->is_some(); }

  private:
    union
    {
        T m_Value;
    };
    bool m_HasValue;

    inline _Ty unwrap_unchecked()
        requires detail::_IsRef<_Ty>::value
    {
        m_HasValue = false;
        return *m_Value;
    }

    inline _Ty&& unwrap_unchecked()
    {
        m_HasValue = false;
        return std::move(m_Value);
    }

    inline _Ty get_unchecked() const
        requires detail::_IsRef<_Ty>::value
    {
        return *m_Value;
    }

    inline _Ty get_unchecked()
        requires detail::_IsRef<_Ty>::value
    {
        return *m_Value;
    }

    inline _Ty const& get_unchecked() const { return m_Value; }
    inline _Ty&       get_unchecked() { return m_Value; }
};
} // namespace cy
