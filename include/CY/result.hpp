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

namespace cy {
template<typename _Ty>
class Ok;

namespace detail {
    template<typename _Ty>
    using _IsRef = std::is_reference<_Ty>;

    template<typename _Ty>
    using _PtrIfRef =
        typename std::conditional<detail::_IsRef<_Ty>::value,
                                  typename std::remove_reference<_Ty>::type *,
                                  _Ty>::type;

    template<typename T>
    constexpr
        typename std::enable_if<_IsRef<T>::value,
                                typename std::remove_reference<T>::type *>::type
        construct_ok(T _value)
    {
        return &_value;
    }

    template<typename T>
    constexpr T&& construct_ok(
        typename std::enable_if<!_IsRef<T>::value, T>::type& _value)
    {
        return std::move(_value);
    }

    struct _ResultConstruct
    {
        _ResultConstruct() = delete;

        template<typename T>
        static constexpr typename std::enable_if<!_IsRef<T>::value, T>::type&&
        construct_ok_result(Ok<T>& ok)
        {
            return std::move(ok.m_Value);
        }

        template<typename T>
        static constexpr typename std::enable_if<
            _IsRef<T>::value,
            typename std::remove_reference<T>::type>::type *
        construct_ok_result(Ok<T>& ok)
        {
            return ok.m_Value;
        }
    };
} // namespace detail

/**
 * @brief Represents an operation that succeeded and returned a value of type
 * `T`.
 *
 */
template<typename _Ty>
class Ok
{
  public:
    using T = detail::_PtrIfRef<_Ty>;

    friend detail::_ResultConstruct;

    explicit constexpr Ok(_Ty value)
        : m_Value(detail::construct_ok<_Ty>(value))
    {
    }

    Ok(Ok const&) = default;
    Ok& operator=(Ok const&) = default;
    Ok(Ok&&) = default;
    Ok& operator=(Ok&&) = default;

    /**
     * @brief Gets the reference owned by `Ok` as a const reference.
     *
     */
    template<typename T = _Ty>
    [[nodiscard]] inline constexpr typename std::enable_if<
        detail::_IsRef<T>::value,
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
        typename std::enable_if<detail::_IsRef<T>::value, T>::type
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
        typename std::enable_if<detail::_IsRef<T>::value, T>::type
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
        typename std::enable_if<!detail::_IsRef<T>::value, T>::type const&
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
        typename std::enable_if<!detail::_IsRef<T>::value, T>::type&
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
        typename std::enable_if<!detail::_IsRef<T>::value, T>::type&&
        take()
    {
        return std::move(m_Value);
    }

  private:
    T m_Value;
};

/**
 * @brief Represents an operation that succeeded and does not return a value.
 *
 */
template<>
class Ok<void>
{
  public:
    explicit constexpr Ok() = default;
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
    static_assert(
        !std::is_reference<E>::value,
        "Result<T, E&> and Err<E&> are invalid! Result must own the error");
    static_assert(!std::is_same<E, void>::value,
                  "Result<T, void> and Err<void> are invalid! Consider "
                  "Maybe<T> instead.");

  public:
    explicit constexpr Err(E err)
        : m_Error(std::move(err))
    {
    }

    Err(Err const&) = default;
    Err& operator=(Err const&) = default;
    Err(Err&&) = default;
    Err& operator=(Err&&) = default;

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

namespace result {
    /**
     * @brief Represents an operation that might fail, returning `Ok` on
     * success, and `Err` on failure.
     *
     */
    template<typename _Ty, typename E>
    class [[nodiscard("Result must be handled")]] Result
    {
        static_assert(
            !std::is_reference<E>::value,
            "Result<T, E&> and Err<E&> are invalid! Result must own the error");
        static_assert(!std::is_same<E, void>::value,
                      "Result<T, void> and Err<void> are invalid! Consider "
                      "Maybe<T> instead.");

      public:
        constexpr Result(Ok<_Ty> ok)
            : m_Value(detail::_ResultConstruct::construct_ok_result<_Ty>(ok))
            , m_IsError(false)
            , m_HasValue(true)
        {
        }

        constexpr Result(Err<E> err)
            : m_Error(err.take())
            , m_IsError(true)
            , m_HasValue(true)
        {
        }

        // FIXME: This destructor should only exist when either T or E aren't
        // trivially destructible.
        ~Result()
        {
            if (m_HasValue && !m_IsError)
                m_Value.~T();
            else if (m_HasValue)
                m_Error.~E();
        }

        explicit Result(Result const&) = default;
        Result& operator=(Result const&) = default;
        Result(Result&&) = default;
        Result& operator=(Result&&) = default;

        /**
         * @brief Whether or not this `Result` contains an error (`Err`).
         *
         * @return true If it does.
         * @return false If it doesn't.
         */
        inline constexpr bool is_error() const { return m_IsError; }
        /**
         * @brief Whether or not this `Result` contains a value (`Ok`).
         *
         * @return true If it does.
         * @return false If it doesn't.
         */
        inline constexpr bool is_ok() const { return !m_IsError; }

        /**
         * @brief Gets a const reference (`T const&`) to the value owned by
         * `Result`. If `Result` owns a reference, the reference is returned as
         * a const reference.
         *
         * @throws cy::get_on_err_error Thrown if `Result` is an error.
         * @throws cy::bad_result Thrown if `Result` does not own a value or
         * reference.
         */
        [[nodiscard]] inline constexpr auto const& get() const&
        {
            if (!m_HasValue)
                throw bad_result();
            if (m_IsError)
                throw get_on_err_error(error_formatter<E>::readable(m_Error));

            return this->get_unchecked();
        }

        /**
         * @brief Gets a reference (`T&`) to the value owned by
         * `Result`. If `Result` owns a reference, that reference is returned.
         *
         * @throws cy::get_on_err_error Thrown if `Result` is an error.
         * @throws cy::bad_result Thrown if `Result` does not own a value or
         * reference.
         */
        [[nodiscard]] inline constexpr auto& get() &
        {
            if (!m_HasValue)
                throw bad_result();
            if (m_IsError)
                throw get_on_err_error(error_formatter<E>::readable(m_Error));

            return this->get_unchecked();
        }

        /**
         * @brief Gets an rvalue reference (`T&&`) to the value owned by
         * `Result`, allowing to move it out of it. If `Result` owns a
         * reference, that reference is moved out.
         *
         * @throws cy::unwrap_on_err_error Thrown if `Result` is an error.
         * @throws cy::bad_result Thrown if `Result` does not own a value or
         * reference.
         */
        [[nodiscard]] inline constexpr auto&& unwrap()
        {
            if (!m_HasValue)
                throw bad_result();
            if (m_IsError)
                throw unwrap_on_err_error(
                    error_formatter<E>::readable(m_Error));

            return this->unwrap_unchecked();
        }

        /**
         * @brief Gets a const reference (`E const&`) to the error owned by
         * `Result`.
         *
         * @throws cy::get_on_ok_error Thrown if `Result` is not an error.
         * @throws cy::bad_result Thrown if `Result` does not own an error.
         */
        [[nodiscard]] inline constexpr E const& get_err() const&
        {
            if (!m_HasValue)
                throw bad_result();
            if (!m_IsError)
                throw get_on_ok_error();

            return this->get_err_unchecked();
        }

        /**
         * @brief Gets a reference (`E&`) to the error owned by
         * `Result`.
         *
         * @throws cy::get_on_ok_error Thrown if `Result` is not an error.
         * @throws cy::bad_result Thrown if `Result` does not own an error.
         */
        [[nodiscard]] inline constexpr E& get_err() &
        {
            if (!m_HasValue)
                throw bad_result();
            if (!m_IsError)
                throw get_on_ok_error();

            return this->get_err_unchecked();
        }

        /**
         * @brief Gets an rvalue reference (`E&&`) to the error owned by
         * `Result`, allowing to move it out.
         *
         * @throws cy::unwrap_on_ok_error Thrown if `Result` is not an error.
         * @throws cy::bad_result Thrown if `Result` does not own an error.
         */
        [[nodiscard]] inline constexpr E&& unwrap_err()
        {
            if (!m_HasValue)
                throw bad_result();
            if (!m_IsError)
                throw unwrap_on_ok_error();

            return this->unwrap_err_unchecked();
        }

        /**
         * @brief Converts this `Result` into a boolean.
         *
         * @return true If the `Result` contains a value (`Result` is `Ok`).
         * @return false If the `Result` contains an error (`Result` is `Err`).
         */
        constexpr operator bool() const { return !m_IsError; }

      private:
        using T = detail::_PtrIfRef<_Ty>;

        union
        {
            T m_Value;
            E m_Error;
        };
        bool m_IsError;
        bool m_HasValue;

        template<typename T = _Ty>
        inline constexpr
            typename std::enable_if<!detail::_IsRef<T>::value, T>::type const&
            get_unchecked() const&
        {
            return m_Value;
        }

        template<typename T = _Ty>
        [[nodiscard]] inline constexpr
            typename std::enable_if<!detail::_IsRef<T>::value, T>::type&
            get_unchecked() &
        {
            return m_Value;
        }

        template<typename T = _Ty>
        [[nodiscard]] inline constexpr typename std::enable_if<
            detail::_IsRef<T>::value,
            typename std::remove_reference<T>::type>::type const&
        get_unchecked() const&
        {
            return *m_Value;
        }

        template<typename T = _Ty>
        [[nodiscard]] inline constexpr
            typename std::enable_if<detail::_IsRef<T>::value, T>::type
            get_unchecked() &
        {
            return *m_Value;
        }

        template<typename T = _Ty>
        [[nodiscard]] inline constexpr
            typename std::enable_if<!detail::_IsRef<T>::value, T>::type&&
            unwrap_unchecked()
        {
            m_HasValue = false;
            return std::move(m_Value);
        }

        template<typename T = _Ty>
        [[nodiscard]] inline constexpr
            typename std::enable_if<detail::_IsRef<T>::value, T>::type
            unwrap_unchecked()
        {
            m_HasValue =
                false; // technically we're not actually moving anything out of
                       // Result, so this could still be true and it should be
                       // fine. But since the behavior is that unwrap moves out
                       // of result, we'll pretend it also does here.
            return *m_Value;
        }

        [[nodiscard]] inline constexpr E const& get_err_unchecked() const&
        {
            return m_Error;
        }
        [[nodiscard]] inline constexpr E& get_err_unchecked() &
        {
            return m_Error;
        }
        [[nodiscard]] inline constexpr E&& unwrap_err_unchecked()
        {
            m_HasValue = false;
            return std::move(m_Error);
        }
    };

    /**
     * @brief Represents an operation that might fail, returning nothing
     * (`Ok<void>`) on success, and `Err` on failure.
     *
     */
    template<typename E>
    class [[nodiscard("Result must be handled")]] Result<void, E>
    {
        static_assert(
            !std::is_reference<E>::value,
            "Result<T, E&> and Err<E&> are invalid! Result must own the error");
        static_assert(!std::is_same<E, void>::value,
                      "Result<T, void> and Err<void> are invalid! Consider "
                      "Maybe<T> instead.");

      public:
        constexpr Result(Ok<void>)
            : m_IsError(false)
            , m_HasError(false)
        {
        }

        constexpr Result(Err<E> err)
            : m_Error(err.take())
            , m_IsError(true)
            , m_HasError(true)
        {
        }

        // FIXME: This destructor should only exist when E is not trivially
        // destructible.
        ~Result()
        {
            if (m_HasError)
                m_Error.~E();
        }

        /**
         * @brief Whether or not this `Result` contains an error (`Err`).
         *
         * @return true If it does.
         * @return false If it doesn't.
         */
        inline constexpr bool is_error() const { return m_IsError; }
        /**
         * @brief Whether or not this `Result` is `Ok`.
         *
         * @return true If it is.
         * @return false If it isn't.
         */
        inline constexpr bool is_ok() const { return !m_IsError; }

        /**
         * @brief "Gets" the void owned by the `Result`. It's really a
         * convenience to simply unwrap `Result`s even if they don't return a
         * vlaue.
         *
         * @throws cy::unwrap_on_err_error Thrown if `Result` is an error.
         */
        inline void unwrap()
        {
            if (m_IsError)
                throw unwrap_on_err_error(
                    error_formatter<E>::readable(m_Error));
        }

        /**
         * @brief Gets a const reference (`E const&`) to the error owned by
         * `Result`.
         *
         * @throws cy::get_on_ok_error Thrown if `Result` is not an error.
         * @throws cy::bad_result Thrown if `Result` does not own an error.
         */
        [[nodiscard]] inline constexpr E const& get_err() const&
        {
            if (!m_HasError)
                throw bad_result();
            if (!m_IsError)
                throw get_on_ok_error();

            return this->get_err_unchecked();
        }

        /**
         * @brief Gets a reference (`E&`) to the error owned by
         * `Result`.
         *
         * @throws cy::get_on_ok_error Thrown if `Result` is not an error.
         * @throws cy::bad_result Thrown if `Result` does not own an error.
         */
        [[nodiscard]] inline constexpr E& get_err() &
        {
            if (!m_HasError)
                throw bad_result();
            if (!m_IsError)
                throw get_on_ok_error();

            return this->get_err_unchecked();
        }

        /**
         * @brief Gets an rvalue reference (`E&&`) to the error owned by
         * `Result`, allowing to move it out.
         *
         * @throws cy::unwrap_on_ok_error Thrown if `Result` is not an error.
         * @throws cy::bad_result Thrown if `Result` does not own an error.
         */
        [[nodiscard]] inline constexpr E&& unwrap_err()
        {
            if (!m_HasError)
                throw bad_result();
            if (!m_IsError)
                throw unwrap_on_ok_error();

            return this->unwrap_err_unchecked();
        }

        /**
         * @brief Converts this `Result` into a boolean.
         *
         * @return true If the `Result` is not an error (`Result` is `Ok`).
         * @return false If the `Result` contains an error (`Result` is `Err`).
         */
        constexpr operator bool() const { return !m_IsError; }

      private:
        union
        {
            E m_Error;
        };
        bool m_IsError;
        bool m_HasError;

        [[nodiscard]] inline constexpr E const& get_err_unchecked() const&
        {
            return m_Error;
        }
        [[nodiscard]] inline constexpr E& get_err_unchecked() &
        {
            return m_Error;
        }
        [[nodiscard]] inline constexpr E&& unwrap_err_unchecked()
        {
            m_HasError = false;
            return std::move(m_Error);
        }
    };
} // namespace result
} // namespace cy
