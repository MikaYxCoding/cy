/**
 * @file safety.hpp
 * @author Jesús Blanco
 * @brief General code safety with value/error and value/none classes.
 * @version 2.0.0
 * @date 2025-06-27
 *
 * @copyright Copyright (c) Jesús Blanco. See LICENSE for details.
 *
 */

#pragma once

#include "core.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <type_traits>

namespace cy {
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
    inline constexpr T const &get() const & { return this->val; }
    /**
     * @brief Gets a reference to `T` (`T&`).
     */
    inline constexpr T &get() & { return this->val; }
    /**
     * @brief Gets an rvalue reference to `T` (`T&&`), allowing to move it out
     * of `Some`.
     */
    inline const T &&take() { return std::move(this->val); }
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

    inline T const &get_unchecked() const & { return this->value; }
    inline T       &get_unchecked()       &{ return this->value; }
    inline T      &&unwrap_unchecked()
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
    constexpr T const &get() const &
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
    constexpr T &get() &
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
    constexpr T &&unwrap()
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
 * @ref Maybe<T&>
 * @ref Some<T>
 */
template<typename T>
class Some<T &>
{
    static_assert(!std::is_void_v<T>, "Maybe<void> is invalid.");

  private:
    T *val;

  public:
    friend Maybe<T &>;

    explicit constexpr Some(T &value)
        : val(&value)
    {
    }

    /**
     * @brief Gets a const reference to `T` (`T const&`).
     *
     */
    inline constexpr T const &get() const & { return *this->val; }
    /**
     * @brief Gets a reference to `T` (`T &`).
     *
     */
    inline constexpr T &get() & { return *this->val; }
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
class Maybe<T &>
{
    static_assert(!std::is_void_v<T>, "Maybe<void> is invalid.");

  private:
    bool has_value;
    T   *value;

    inline T &unwrap_unchecked()
    {
        this->has_value = false;
        return *this->value;
    }

  public:
    constexpr Maybe(Some<T &> some)
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
    constexpr T &unwrap()
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
    Maybe<U> map(std::function<U(T &)> func)
    {
        if (this->has_value) {
            return Some(func(this->unwrap_unchecked()));
        }

        return None();
    }
};

/**
 * @brief An Ok value.
 * @ref Result<T, E>
 */
template<typename T>
class Ok
{
  private:
    T value;

  public:
    explicit constexpr Ok(T v)
        : value(std::move(v))
    {
    }

    /**
     * @brief Gets a const reference to `T` (`T const&`).
     *
     */
    inline T const &get() const & { return this->value; }
    /**
     * @brief Gets a reference to `T` (`T &`).
     *
     */
    inline T &get() & { return this->value; }
    /**
     * @brief Gets an rvalue reference to `T` (`T&&`), allowing to move it out
     * of `Ok`.
     */
    inline T &&take() { return std::move(this->value); }
};
/**
 * @brief An Err value.
 * @ref Result<T, E>
 */
template<typename E>
class Err
{
    static_assert(!std::is_void_v<E>,
                  "Result<T, void> is invalid. Use Maybe<T> instead.");

  private:
    E err;

  public:
    explicit constexpr Err(E error)
        : err(std::move(error))
    {
    }

    /**
     * @brief Gets a const reference to `E` (`E const&`).
     *
     */
    inline E const &get() const & { return this->err; }
    /**
     * @brief Gets a reference to `E` (`E &`).
     *
     */
    inline E &get() & { return this->err; }
    /**
     * @brief Gets an rvalue reference to `E` (`E&&`), allowing to move it out
     * of `Err`.
     */
    inline E &&take() { return std::move(this->err); }
};

#define CHECK_IF_VALID_T(func)                                                 \
    if (this->is_error)                                                        \
        throw std::runtime_error("Called " func " on an error value");         \
    if (!this->has_data)                                                       \
        throw std::runtime_error(                                              \
            "Called " func                                                     \
            " on a moved value (Result had T, but was unwrapped)");

#define CHECK_IF_VALID_E(func)                                                 \
    if (!this->is_error)                                                       \
        throw std::runtime_error("Called " func " on an ok value");            \
    if (!this->has_data)                                                       \
        throw std::runtime_error(                                              \
            "Called " func                                                     \
            " on a moved value (Result had E, but was unwrapped)");

/**
 * @brief Class representing a value/error situation.
 * @ref Ok<T>
 * @ref Err<E>
 */
template<typename T, typename E>
class [[nodiscard("Result must be handled.")]] Result
{
    static_assert(!std::is_void_v<E>,
                  "Result<T, void> is invalid. Use Maybe<T> instead.");

  private:
    bool is_error;
    bool has_data;

    union
    {
        T value;
        E error;
    };

    inline T const &get_unchecked() const & { return this->value; }
    inline T       &get_unchecked()       &{ return this->value; }

    inline E const &get_err_unchecked() const & { return this->error; }
    inline E       &get_err_unchecked()       &{ return this->error; }

    inline T &&unwrap_unchecked()
    {
        this->has_data = false;
        return std::move(this->value);
    }

    inline E &&unwrap_err_unchecked()
    {
        this->has_data = false;
        return std::move(this->error);
    }

  public:
    ~Result()
    {
        if (this->is_error && this->has_data)
            this->error.~E();
        else if (this->has_data)
            this->value.~T();
    }

    constexpr Result(Ok<T> ok)
        : is_error(false)
        , has_data(true)
        , value(ok.take())
    {
    }

    constexpr Result(Err<E> err)
        : is_error(true)
        , has_data(true)
        , error(err.take())
    {
    }

    /**
     * @brief Whether this `Result<T, E>` is `Err<E>`.
     *
     * @return true If it is.
     * @return false If it isn't.
     */
    inline constexpr bool is_err() const { return this->is_error; }
    /**
     * @brief Opposite of `is_err`.
     */
    inline constexpr bool is_ok() const { return !this->is_err(); }

    /**
     * @brief Gets a const reference to `T` (`T const&`).
     *
     * @exception std::runtime_error Thrown if `Result<T, E>` doesn't actually
     * have a value.
     */
    constexpr T const &get() const &
    {
        CHECK_IF_VALID_T(".get()");

        return this->get_unchecked();
    }

    /**
     * @brief Gets a reference to `T` (`T&`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have a value.
     */
    constexpr T &get() &
    {
        CHECK_IF_VALID_T(".get()");

        return this->get_unchecked();
    }

    /**
     * @brief Gets a const reference to `E` (`E const&`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have an error.
     */
    constexpr E const &get_err() const &
    {
        CHECK_IF_VALID_E(".get_err()");

        return this->get_err_unchecked();
    }

    /**
     * @brief Gets a  reference to `E` (`E &`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have an error.
     */
    constexpr E &get_err() &
    {
        CHECK_IF_VALID_E(".get_err()");

        return this->get_err_unchecked();
    }

    /**
     * @brief Unwraps the value in `Result<T, E>`, allowing to move it out.
     *
     * @exception std::runtime_error Thrown if `Result<T, E>` doesn't actually
     * have a value.
     */
    constexpr T &&unwrap()
    {
        CHECK_IF_VALID_T(".unwrap()");

        return this->unwrap_unchecked();
    }

    /**
     * @brief Unwraps the error in `Result<T, E>`, allowing to move it out.
     *
     * @exception std::runtime_error Thrown if `Result<T, E>` doesn't actually
     * have an error.
     */
    constexpr E &&unwrap_err()
    {
        CHECK_IF_VALID_E(".unwrap_err()");

        return this->unwrap_err_unchecked();
    }

    /**
     * @brief Converts this result into a `Maybe<T>`, which will be `Some<T>` if
     * `Result<T, E>` is `Ok<T>` and holds a valid `T`. `T` will me moved into
     * `Maybe<T>`. Otherwise, `None`.
     */
    constexpr Maybe<T> ok()
    {
        if (!this->is_error && this->has_data) {
            return Some<T>(this->unwrap_unchecked());
        }

        return None();
    }

    /**
     * @brief Converts this result into a `Maybe<E>`, which will be `Some<E>` if
     * `Result<T, E>` is `Err<E>` and holds a valid `E`. `E` will me moved into
     * `Maybe<E>`. Otherwise, `None`.
     */
    constexpr Maybe<E> err()
    {
        if (this->is_error && this->has_data) {
            return Some<E>(this->unwrap_err_unchecked());
        }

        return None();
    }
};

/**
 * @attention This is a template specialization for `T&`. Working with
 * references directly is tricky, so both `Ok<T&>` and `Result<T&, E>` hold `T*`
 * instead of an actual reference.
 * @brief An Ok value.
 * @ref Result<T, E>
 */
template<typename T>
class Ok<T &>
{
  private:
    T *value;

  public:
    explicit constexpr Ok(T &v)
        : value(&v)
    {
    }

    /**
     * @brief Gets the reference to `T` (`T&`).
     */
    inline T &take() { return *this->value; }
};

template<typename T, typename E>
class [[nodiscard("Result must be handled.")]] Result<T &, E>
{
    static_assert(!std::is_void_v<E>,
                  "Result<T, void> is invalid. Use Maybe<T> instead.");

  private:
    bool is_error;
    bool has_data;

    union
    {
        T *value;
        E  error;
    };

    inline E const &get_err_unchecked() const & { return this->error; }
    inline E       &get_err_unchecked()       &{ return this->error; }

    inline T &unwrap_unchecked()
    {
        this->has_data = false;
        return *this->value;
    }

    inline E &&unwrap_err_unchecked()
    {
        this->has_data = false;
        return std::move(this->error);
    }

  public:
    ~Result()
    {
        if (this->is_error && this->has_data)
            this->error.~E();
    }

    constexpr Result(Ok<T &> ok)
        : is_error(false)
        , has_data(true)
        , value(&ok.take())
    {
    }

    constexpr Result(Err<E> err)
        : is_error(true)
        , has_data(true)
        , error(err.take())
    {
    }

    /**
     * @brief Whether this `Result<T&, E>` is `Err<E>`.
     *
     * @return true If it is.
     * @return false If it isn't.
     */
    inline constexpr bool is_err() const { return this->is_error; }
    /**
     * @brief Opposite of `is_err`.
     */
    inline constexpr bool is_ok() const { return !this->is_err(); }

    /**
     * @brief Gets a const reference to `E` (`E const&`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have an error.
     */
    constexpr E const &get_err() const &
    {
        CHECK_IF_VALID_E(".get_err()");

        return this->get_err_unchecked();
    }

    /**
     * @brief Gets a reference to `E` (`E &`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have an error.
     */
    constexpr E &get_err() &
    {
        CHECK_IF_VALID_E(".get_err()");

        return this->get_err_unchecked();
    }

    /**
     * @brief Unwraps the reference in `Result<T&, E>`, allowing to move it out.
     *
     * @exception std::runtime_error Thrown if `Result<T&, E>` doesn't actually
     * have a reference.
     */
    constexpr T &unwrap()
    {
        CHECK_IF_VALID_T(".unwrap()");

        return this->unwrap_unchecked();
    }

    /**
     * @brief Unwraps the error in `Result<T, E>`, allowing to move it out.
     *
     * @exception std::runtime_error Thrown if `Result<T, E>` doesn't actually
     * have an error.
     */
    constexpr E &&unwrap_err()
    {
        CHECK_IF_VALID_E(".unwrap_err()");

        return this->unwrap_err_unchecked();
    }

    /**
     * @brief Converts this result into a `Maybe<T>`, which will be `Some<T>` if
     * `Result<T, E>` is `Ok<T>` and holds a valid `T`. `T` will me moved into
     * `Maybe<T>`. Otherwise, `None`.
     */
    constexpr Maybe<T &> ok()
    {
        if (!this->is_error && this->has_data) {
            return Some<T &>(this->unwrap_unchecked());
        }

        return None();
    }

    /**
     * @brief Converts this result into a `Maybe<E>`, which will be `Some<E>` if
     * `Result<T, E>` is `Err<E>` and holds a valid `E`. `E` will me moved into
     * `Maybe<E>`. Otherwise, `None`.
     */
    constexpr Maybe<E> err()
    {
        if (this->is_error && this->has_data) {
            return Some<E>(this->unwrap_err_unchecked());
        }

        return None();
    }
};

/**
 * @attention This is a template specialization for `void`.
 * @brief A succeeded operation that doesn't actually return a value..
 * @ref Result<T, E>
 */
template<>
class Ok<void>
{
  public:
    constexpr Ok() = default;
};
Ok() -> Ok<void>;

template<typename E>
class [[nodiscard("Result must be handled.")]] Result<void, E>
{
    static_assert(!std::is_void_v<E>,
                  "Result<T, void> is invalid. Use Maybe<T> instead.");

  private:
    bool is_error;
    bool has_data;

    union
    {
        E error;
    };

    inline E const &get_err_unchecked() const & { return this->error; }
    inline E       &get_err_unchecked()       &{ return this->error; }

    inline E &&unwrap_err_unchecked()
    {
        this->has_data = false;
        return std::move(this->error);
    }

  public:
    ~Result()
    {
        if (this->is_error && this->has_data)
            this->error.~E();
    }

    constexpr Result(Ok<void>)
        : is_error(false)
        , has_data(true)
    {
    }

    constexpr Result(Err<E> err)
        : is_error(true)
        , has_data(true)
        , error(err.take())
    {
    }

    /**
     * @brief Whether this `Result<T, E>` is `Err<E>`.
     *
     * @return true If it is.
     * @return false If it isn't.
     */
    inline constexpr bool is_err() const { return this->is_error; }
    /**
     * @brief Opposite of `is_err`.
     */
    inline constexpr bool is_ok() const { return !this->is_err(); }

    /**
     * @brief Gets a const reference to `E` (`E const&`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have an error.
     */
    constexpr E const &get_err() const &
    {
        CHECK_IF_VALID_E(".get_err()");

        return this->get_err_unchecked();
    }

    /**
     * @brief Gets a reference to `E` (`E &`).
     *
     * @exception std::runtime_error Thrown if `Result<T,E>` doesn't actually
     * have an error.
     */
    constexpr E &get_err() &
    {
        CHECK_IF_VALID_E(".get_err()");

        return this->get_err_unchecked();
    }

    /**
     * @brief Unwraps the error in `Result<T, E>`, allowing to move it out.
     *
     * @exception std::runtime_error Thrown if `Result<T, E>` doesn't actually
     * have an error.
     */
    constexpr E &&unwrap_err()
    {
        CHECK_IF_VALID_E(".unwrap_err()");

        return this->unwrap_err_unchecked();
    }

    /**
     * @brief Converts this result into a `Maybe<E>`, which will be `Some<E>` if
     * `Result<T, E>` is `Err<E>` and holds a valid `E`. `E` will me moved into
     * `Maybe<E>`. Otherwise, `None`.
     */
    constexpr Maybe<E> err()
    {
        if (this->is_error && this->has_data) {
            return Some(this->unwrap_err_unchecked());
        }

        return None();
    }
};
}