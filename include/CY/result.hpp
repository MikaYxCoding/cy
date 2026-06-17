/**
 * @file result.hpp
 * @author Jesús Blanco
 * @brief Result implementation.
 * @version 4.0.0
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

/**
 * @brief Represents an operation that succeeded and returned a value of type
 * `T`.
 *
 */
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

/**
 * @brief Represents an operation that succeeded and does not return a value.
 *
 */
template<>
class Ok<void>
{
  public:
    constexpr explicit Ok() = default;
};

Ok() -> Ok<void>;

/**
 * @brief Represents an operation that failed and returned an error of type
 * `E`.
 *
 */
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
    /**
     * @brief Represents an operation that might fail, returning `Ok` on
     * success, and `Err` on failure.
     *
     */
    template<typename _Ty, typename _Ety>
    class [[nodiscard("result must be handled")]] Result
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
                return *this;
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
                return *this;
            }

            m_HasValue = true;
            m_IsError = source.m_IsError;
            if (m_IsError)
                m_Error = std::move(source.m_Error);
            else
                m_Value = std::move(source.m_Value);

            return *this;
        }

        /**
         * @brief Whether this result is `Ok`, meaning the opeartion succeeded
         * and returned a value.
         *
         */
        constexpr bool is_ok() const { return !m_IsError; }
        /**
         * @brief Whether this result is `Err`, meaning the operation failed and
         * returned an error.
         *
         */
        constexpr bool is_error() const { return m_IsError; }

        /**
         * @brief Whether this result is `Ok` and the value inside matches a
         * predicate `f`.
         *
         * @param f The predicate to evaluate.
         */
        inline bool is_ok_and(std::function<bool(_Ty)> f) const
            requires detail::_IsRef<_Ty>::value
        {
            if (this->is_error())
                return false;

            return f(*m_Value);
        }
        /**
         * @brief Whether this result is `Ok` and the value inside matches a
         * predicate `f`.
         *
         * @param f The predicate to evaluate.
         */
        inline bool is_ok_and(std::function<bool(_Ty const&)> f) const
        {
            if (this->is_error())
                return false;

            return f(m_Value);
        }
        /**
         * @brief Whether this result is `Err` and the error inside matches a
         * predicate `f`.
         *
         * @param f The predicate to evaluate.
         */
        inline bool is_err_and(std::function<bool(_Ety)> f) const
            requires detail::_IsRef<_Ety>::value
        {
            if (this->is_ok())
                return false;

            return f(*m_Error);
        }
        /**
         * @brief Whether this result is `Err` and the error inside matches a
         * predicate `f`.
         *
         * @param f The predicate to evaluate.
         */
        inline bool is_err_and(std::function<bool(_Ety const&)> f) const
        {
            if (this->is_ok())
                return false;

            return f(m_Error);
        }
        /**
         * @brief Returns the contained `Ok` value, allowing to move it out of
         * the `Result`. Since this function moves out the value, it should only
         * be called once.
         *
         * @exception cy::unwrap_on_err_error Thrown if the `Result` does not
         * contain an `Ok` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& unwrap()
        {
            if (this->is_error())
                throw cy::unwrap_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->unwrap_unchecked();
        }
        /**
         * @brief Returns the contained `Err` value, allowing to move it out of
         * the `Result`. Since this function moves out the value, it should only
         * be called once.
         *
         * @exception cy::unwrap_on_ok_error Thrown if the `Result` does not
         * contain an `Err` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& unwrap_err()
        {
            if (this->is_ok())
                throw unwrap_on_ok_error();
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->unwrap_err_unchecked();
        }
        /**
         * @brief Returns a const reference to the contained `Ok` value. This
         * function does not move anything and it's perfectly safe to call
         * multiple times.
         *
         * @exception cy::get_on_err_error Thrown if the `Result` does not
         * contain an `Ok` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& get() const
        {
            if (this->is_error())
                throw cy::get_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->get_unchecked();
        }
        /**
         * @brief Returns a reference to the contained `Ok` value. This
         * function does not move anything and it's perfectly safe to call
         * multiple times.
         *
         * @exception cy::get_on_err_error Thrown if the `Result` does not
         * contain an `Ok` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& get()
        {
            if (this->is_error())
                throw cy::get_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->get_unchecked();
        }
        /**
         * @brief Returns a const reference to the contained `Err` value. This
         * function does not move anything and it's perfectly safe to call
         * multiple times.
         *
         * @exception cy::get_on_ok_error Thrown if the `Result` does not
         * contain an `Err` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& get_err() const
        {
            if (this->is_ok())
                throw cy::get_on_ok_error();
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->get_err_unchecked();
        }
        /**
         * @brief Returns a reference to the contained `Ok` value. This
         * function does not move anything and it's perfectly safe to call
         * multiple times.
         *
         * @exception cy::get_on_err_error Thrown if the `Result` does not
         * contain an `Ok` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& get_err()
        {
            if (this->is_ok())
                throw cy::get_on_ok_error();
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->get_err_unchecked();
        }
        /**
         * @brief Converts this `Result` into a boolean. Equivalent to
         * `Result::is_ok`.
         *
         */
        [[nodiscard]] constexpr operator bool() const { return this->is_ok(); }

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

    /**
     * @brief Represents an operation that might fail, returning nothing
     * (`Ok<void>`) on success, and `Err` on failure.
     *
     */
    template<typename _Ety>
    class [[nodiscard("result must be handled")]] Result<void, _Ety>
    {
        static_assert(!std::is_same<_Ety, void>::value,
                      "Result<T, void> is invalid! Use Maybe<T> instead");

        using E = detail::_PtrIfRef<_Ety>;

      public:
        constexpr Result(Ok<void>)
            : m_HasValue(false)
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
            requires std::is_trivially_destructible<_Ety>::value
        = default;

        constexpr ~Result()
            requires(!std::is_trivially_destructible<_Ety>::value)
        {
            if (m_HasValue)
                m_Error.~_Ety();
        }

        Result(Result const&)
            requires std::is_trivially_copyable<_Ety>::value
        = default;

        Result& operator=(Result const&)
            requires std::is_trivially_copyable<_Ety>::value
        = default;

        Result(Result&&)
            requires std::is_trivially_move_constructible<_Ety>::value
        = default;

        Result& operator=(Result&&)
            requires std::is_trivially_move_assignable<_Ety>::value
        = default;

        Result(Result const& other)
        {
            if (!other.m_HasValue) {
                m_HasValue = false;
                m_IsError = false;
                return;
            }

            m_HasValue = true;
            m_IsError = true;
            m_Error = other.m_Error;
        }

        Result& operator=(Result const& other)
        {
            if (!other.m_HasValue) {
                m_HasValue = false;
                m_IsError = false;
                return *this;
            }

            m_HasValue = true;
            m_IsError = true;
            m_Error = other.m_Error;

            return *this;
        }

        Result(Result&& source)
        {
            if (!source.m_HasValue) {
                m_HasValue = false;
                m_IsError = false;
                return;
            }

            m_HasValue = true;
            m_IsError = true;
            m_Error = std::move(source.m_Error);
        }

        Result& operator=(Result&& source)
        {
            if (this == &source)
                return *this;

            if (!source.m_HasValue) {
                m_HasValue = false;
                m_IsError = false;
                return *this;
            }

            m_HasValue = true;
            m_IsError = true;
            m_Error = std::move(source.m_Error);

            return *this;
        }

        /**
         * @brief Whether this result is `Ok`, meaning the opeartion succeeded.
         *
         */
        constexpr bool is_ok() const { return !m_IsError; }
        /**
         * @brief Whether this result is `Err`, meaning the opeartion failed
         * and returned an error.
         *
         */
        constexpr bool is_error() const { return m_IsError; }
        /**
         * @brief Whether this result is `Err` and the error inside matches a
         * predicate `f`.
         *
         * @param f The predicate to evaluate.
         */
        inline bool is_err_and(std::function<bool(_Ety)> f) const
            requires detail::_IsRef<_Ety>::value
        {
            if (this->is_ok())
                return false;

            return f(*m_Error);
        }
        /**
         * @brief Whether this result is `Err` and the error inside matches a
         * predicate `f`.
         *
         * @param f The predicate to evaluate.
         */
        inline bool is_err_and(std::function<bool(_Ety const&)> f) const
        {
            if (this->is_ok())
                return false;

            return f(m_Error);
        }
        /**
         * @brief Returns the contained void if the result is `Ok` and throws an
         * exception if the `Result` is `Err`.
         *
         * @exception cy::unwrap_on_err_error Thrown if the `Result` does not
         * contain an `Ok` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        inline void unwrap()
        {
            if (this->is_error())
                throw cy::unwrap_on_err_error(
                    cy::error_formatter<typename std::remove_reference<
                        _Ety>::type>::readable(this->get_err_unchecked()));
        }
        /**
         * @brief Returns the contained `Err` value, allowing to move it out of
         * the `Result`. Since this function moves out the value, it should only
         * be called once.
         *
         * @exception cy::unwrap_on_ok_error Thrown if the `Result` does not
         * contain an `Err` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& unwrap_err()
        {
            if (this->is_ok())
                throw unwrap_on_ok_error();
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->unwrap_err_unchecked();
        }
        /**
         * @brief Returns a const reference to the contained `Err` value. This
         * function does not move anything and it's perfectly safe to call
         * multiple times.
         *
         * @exception cy::get_on_ok_error Thrown if the `Result` does not
         * contain an `Err` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& get_err() const
        {
            if (this->is_ok())
                throw cy::get_on_ok_error();
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->get_err_unchecked();
        }
        /**
         * @brief Returns a reference to the contained `Err` value. This
         * function does not move anything and it's perfectly safe to call
         * multiple times.
         *
         * @exception cy::get_on_ok_error Thrown if the `Result` does not
         * contain an `Err` value.
         * @exception cy::bad_result Throw if the `Result` no longers contains
         * the value (it was has already been unwrapped).
         *
         */
        [[nodiscard]] inline auto&& get_err()
        {
            if (this->is_ok())
                throw cy::get_on_ok_error();
            if (!this->m_HasValue)
                throw cy::bad_result();

            return this->get_err_unchecked();
        }
        /**
         * @brief Converts this `Result` into a boolean. Equivalent to
         * `Result::is_ok`.
         *
         */
        [[nodiscard]] constexpr operator bool() const { return this->is_ok(); }

      private:
        union
        {
            E m_Error;
        };
        bool m_HasValue;
        bool m_IsError;

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

/**
 * @brief Represents an operation that might fail. It returns `Ok` on success,
 * and `Err` on failure. This is a template specialization for the most common
 * use case of `Result<T, str>` (the error type is just an string error
 * message). If you want to return some other error type, use
 * cy::result::Result, and specialize cy::error_formatter for you error
 * type.
 */
template<typename T>
using Result = result::Result<T, str>;
} // namespace cy
