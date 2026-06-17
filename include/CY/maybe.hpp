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

class None
{
  public:
    explicit constexpr None() = default;
};

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

    constexpr bool is_some() const { return m_HasValue; }
    constexpr bool is_none() const { return !m_HasValue; }

    inline bool is_some_and(std::function<bool(_Ty)> f) const
        requires detail::_IsRef<_Ty>::value
    {
        if (this->is_none())
            return false;

        return f(*m_Value);
    }

    inline bool is_some_and(std::function<bool(_Ty const&)> f) const
    {
        if (this->is_none())
            return false;

        return f(m_Value);
    }

    inline auto&& unwrap()
    {
        if (this->is_none())
            throw cy::get_none_error();

        return this->unwrap_unchecked();
    }

    template<typename U>
    inline Maybe<U> map(std::function<U(_Ty)> f)
    {
        if (this->is_none())
            return None();

        return Some<U>(f(this->unwrap_unchecked()));
    }

    constexpr operator bool() const { return this->is_some(); }

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
};
} // namespace cy
