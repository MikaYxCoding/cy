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

template<typename T>
class Ok
{
  public:
    T value;

    constexpr explicit Ok(T _value)
        requires detail::_IsRef<T>::value
        : value(_value)
    {
    }

    constexpr explicit Ok(T _value)
        : value(std::move(_value))
    {
    }
};

template<typename E>
class Err
{
  public:
    E err;

    constexpr explicit Err(E _err)
        requires detail::_IsRef<E>::value
        : err(_err)
    {
    }

    constexpr explicit Err(E _err)
        : err(std::move(_err))
    {
    }
};

namespace result {
    template<typename _Ty, typename _Ety>
    class Result
    {
        static_assert(!std::is_same<_Ety, void>::value,
                      "Result<T, void> is invalid! Use Maybe<T> instead");

        using T = detail::_PtrIfRef<_Ty>;
        using E = detail::_PtrIfRef<_Ety>;

      public:
        constexpr Result(Ok<_Ty> ok)
            requires detail::_IsRef<_Ty>::value
            : m_Value(&ok.value)
            , m_HasValue(true)
            , m_IsError(false)
        {
        }

        constexpr Result(Ok<_Ty> ok)
            : m_Value(std::move(ok.value))
            , m_HasValue(true)
            , m_IsError(false)
        {
        }

        constexpr Result(Err<_Ety> err)
            requires detail::_IsRef<_Ety>::value
            : m_Error(&err.err)
            , m_HasValue(true)
            , m_IsError(true)
        {
        }

        constexpr Result(Err<_Ety> err)
            : m_Error(std::move(err.err))
            , m_HasValue(true)
            , m_IsError(true)
        {
        }

        constexpr ~Result()
            requires std::is_trivially_destructible<_Ty>::value
                         && std::is_trivially_destructible<_Ety>::value
        = default;

        constexpr ~Result()
            requires(!std::is_trivially_destructible<_Ty>::value &&
                     std::is_trivially_destructible<_Ety>::value)
        {
            if (m_HasValue && !m_IsError)
                m_Value.~_Ty();
        }

        constexpr ~Result()
            requires(std::is_trivially_destructible<_Ty>::value &&
                     !std::is_trivially_destructible<_Ety>::value)
        {
            if (m_HasValue && m_IsError)
                m_Error.~_Ety();
        }

        constexpr ~Result()
        {
            if (m_HasValue && m_IsError)
                m_Error.~_Ety();
            else if (m_HasValue)
                m_Value.~_Ty();
        }

        Result(Result const&)
            requires std::is_trivially_copyable<_Ty>::value
                         && std::is_trivially_copyable<_Ety>::value
        = default;

        Result& operator=(Result const&)
            requires std::is_trivially_copyable<_Ty>::value
                         && std::is_trivially_copyable<_Ety>::value
        = default;

        Result(Result&&)
            requires std::is_trivially_move_constructible<_Ty>::value
                         && std::is_trivially_move_constructible<_Ety>::value
        = default;

        Result& operator=(Result&&)
            requires std::is_trivially_move_assignable<_Ty>::value
                         && std::is_trivially_move_assignable<_Ety>::value
        = default;

        Result(Result const& other)
        {
            if (!other.m_HasValue) {
                m_HasValue = false;
                m_IsError = other.m_IsError;
                return;
            }

            m_HasValue = true;
            m_IsError = other.m_IsError;
            if (m_IsError)
                m_Error = other.m_Error;
            else
                m_Value = other.m_Value;
        }

        Result& operator=(Result const& other)
        {
            if (!other.m_HasValue) {
                m_HasValue = false;
                m_IsError = other.m_IsError;
                return;
            }

            m_HasValue = true;
            m_IsError = other.m_IsError;
            if (m_IsError)
                m_Error = other.m_Error;
            else
                m_Value = other.m_Value;

            return *this;
        }

        Result(Result&& source)
        {
            if (!source.m_HasValue) {
                m_HasValue = false;
                m_IsError = source.m_IsError;
                return;
            }

            m_HasValue = true;
            m_IsError = source.m_IsError;
            if (m_IsError)
                m_Error = std::move(source.m_Error);
            else
                m_Value = std::move(source.m_Value);
        }

        Result& operator=(Result&& source)
        {
            if (this == &source)
                return *this;

            if (!source.m_HasValue) {
                m_HasValue = false;
                m_IsError = source.m_IsError;
                return;
            }

            m_HasValue = true;
            m_IsError = source.m_IsError;
            if (m_IsError)
                m_Error = std::move(source.m_Error);
            else
                m_Value = std::move(source.m_Value);

            return *this;
        }

        constexpr bool is_ok() const { return !m_IsError; }
        constexpr bool is_error() const { return m_IsError; }

        inline bool is_ok_and(std::function<bool(_Ty)> f) const
            requires detail::_IsRef<_Ty>::value
        {
            if (this->is_error())
                return false;

            return f(*m_Value);
        }

        inline bool is_ok_and(std::function<bool(_Ty const&)> f) const
        {
            if (this->is_error())
                return false;

            return f(m_Value);
        }

        inline bool is_err_and(std::function<bool(_Ety)> f) const
            requires detail::_IsRef<_Ety>::value
        {
            if (this->is_ok())
                return false;

            return f(*m_Error);
        }

        inline bool is_err_and(std::function<bool(_Ety const&)> f) const
        {
            if (this->is_ok())
                return false;

            return f(m_Error);
        }

        inline auto&& unwrap()
        {
            if (this->is_error())
                throw cy::unwrap_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));

            return this->unwrap_unchecked();
        }

        inline auto&& unwrap_err()
        {
            if (this->is_ok())
                throw unwrap_on_ok_error();

            return this->unwrap_err_unchecked();
        }

        inline auto&& get() const
        {
            if (this->is_error())
                throw cy::get_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));

            return this->get_unchecked();
        }

        inline auto&& get()
        {
            if (this->is_error())
                throw cy::get_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));

            return this->get_unchecked();
        }

        inline auto&& get_err() const
        {
            if (this->is_ok())
                throw cy::get_on_ok_error();

            return this->get_err_unchecked();
        }

        inline auto&& get_err()
        {
            if (this->is_ok())
                throw cy::get_on_ok_error();

            return this->get_err_unchecked();
        }

        constexpr operator bool() const { return this->is_ok(); }

      private:
        union
        {
            T m_Value;
            E m_Error;
        };
        bool m_HasValue;
        bool m_IsError;

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

        inline _Ety unwrap_err_unchecked()
            requires detail::_IsRef<_Ety>::value
        {
            m_HasValue = false;
            return *m_Error;
        }

        inline _Ety&& unwrap_err_unchecked()
        {
            m_HasValue = false;
            return std::move(m_Error);
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

        inline _Ety get_err_unchecked() const
            requires detail::_IsRef<_Ety>::value
        {
            return *m_Error;
        }

        inline _Ety get_err_unchecked()
            requires detail::_IsRef<_Ety>::value
        {
            return *m_Error;
        }

        inline _Ety const& get_err_unchecked() const { return m_Error; }
        inline _Ety&       get_err_unchecked() { return m_Error; }
    };
} // namespace result

template<typename T>
using Result = result::Result<T, str>;
} // namespace cy
