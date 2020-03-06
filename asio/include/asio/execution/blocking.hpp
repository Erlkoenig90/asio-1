//
// execution/blocking.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_BLOCKING_HPP
#define ASIO_EXECUTION_BLOCKING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution/executor.hpp"
#include "asio/is_applicable_property.hpp"
#include "asio/query.hpp"
#include "asio/traits/prefer_free.hpp"
#include "asio/traits/query_free.hpp"
#include "asio/traits/query_member.hpp"
#include "asio/traits/query_static_constexpr_member.hpp"
#include "asio/traits/require_free.hpp"
#include "asio/traits/require_member.hpp"
#include "asio/traits/static_query.hpp"
#include "asio/traits/static_require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace execution {
namespace detail {
namespace blocking {

template <int I> struct possibly_t;
template <int I> struct always_t;
template <int I> struct never_t;

} // namespace blocking

template <int I = 0>
struct blocking_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = false);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = false);
  typedef blocking_t polymorphic_query_result_type;

  typedef detail::blocking::possibly_t<I> possibly_t;
  typedef detail::blocking::always_t<I> always_t;
  typedef detail::blocking::never_t<I> never_t;

  ASIO_CONSTEXPR blocking_t()
    : value_(-1)
  {
  }

  ASIO_CONSTEXPR blocking_t(possibly_t)
    : value_(0)
  {
  }

  ASIO_CONSTEXPR blocking_t(always_t)
    : value_(1)
  {
  }

  ASIO_CONSTEXPR blocking_t(never_t)
    : value_(2)
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, blocking_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, blocking_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, blocking_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, possibly_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, blocking_t>::is_valid
          && !traits::query_member<T, blocking_t>::is_valid
          && traits::static_query<T, possibly_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, possibly_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, always_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, blocking_t>::is_valid
          && !traits::query_member<T, blocking_t>::is_valid
          && !traits::static_query<T, possibly_t>::is_valid
          && traits::static_query<T, always_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, always_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, never_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, blocking_t>::is_valid
          && !traits::query_member<T, blocking_t>::is_valid
          && !traits::static_query<T, possibly_t>::is_valid
          && !traits::static_query<T, always_t>::is_valid
          && traits::static_query<T, never_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, never_t>::value();
  }

  template <typename E, typename T = decltype(blocking_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = blocking_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  friend ASIO_CONSTEXPR bool operator==(
      const blocking_t& a, const blocking_t& b)
  {
    return a.value_ == b.value_;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const blocking_t& a, const blocking_t& b)
  {
    return a.value_ != b.value_;
  }

  struct convertible_from_blocking_t
  {
    ASIO_CONSTEXPR convertible_from_blocking_t(blocking_t) {}
  };

  template <typename Executor>
  friend ASIO_CONSTEXPR blocking_t query(
      const Executor& ex, convertible_from_blocking_t,
      typename enable_if<
        can_query<const Executor&, possibly_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, blocking_t<>::possibly_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, possibly_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, possibly_t());
  }

  template <typename Executor>
  friend ASIO_CONSTEXPR blocking_t query(
      const Executor& ex, convertible_from_blocking_t,
      typename enable_if<
        !can_query<const Executor&, possibly_t>::value
          && can_query<const Executor&, always_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, blocking_t<>::always_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, always_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, always_t());
  }

  template <typename Executor>
  friend ASIO_CONSTEXPR blocking_t query(
      const Executor& ex, convertible_from_blocking_t,
      typename enable_if<
        !can_query<const Executor&, possibly_t>::value
          && !can_query<const Executor&, always_t>::value
          && can_query<const Executor&, never_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, blocking_t<>::never_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, never_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, never_t());
  }

  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(possibly_t, possibly);
  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(always_t, always);
  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(never_t, never);

#if !defined(ASIO_HAS_CONSTEXPR)
  static const blocking_t instance;
#endif // !defined(ASIO_HAS_CONSTEXPR)

private:
  int value_;
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T blocking_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

#if !defined(ASIO_HAS_CONSTEXPR)
template <int I>
const blocking_t<I> blocking_t<I>::instance;
#endif

template <int I>
const typename blocking_t<I>::possibly_t blocking_t<I>::possibly;

template <int I>
const typename blocking_t<I>::always_t blocking_t<I>::always;

template <int I>
const typename blocking_t<I>::never_t blocking_t<I>::never;

namespace blocking {

template <int I = 0>
struct possibly_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef blocking_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR possibly_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, possibly_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, possibly_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, possibly_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR possibly_t static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, possibly_t>::is_valid
          && !traits::query_member<T, possibly_t>::is_valid
          && !traits::query_free<T, possibly_t>::is_valid
          && !can_query<T, always_t<I> >::value
          && !can_query<T, never_t<I> >::value
      >::type* = 0) ASIO_NOEXCEPT
  {
    return possibly_t();
  }

  template <typename E, typename T = decltype(possibly_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = possibly_t::static_query<E>();
#endif // defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR blocking_t<I> value()
  {
    return possibly_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const possibly_t&, const possibly_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const possibly_t&, const possibly_t&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const possibly_t&, const always_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const possibly_t&, const always_t<I>&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const possibly_t&, const never_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const possibly_t&, const never_t<I>&)
  {
    return true;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T possibly_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <int I = 0>
struct always_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef blocking_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR always_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, always_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, always_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, always_t>::value();
  }

  template <typename E, typename T = decltype(always_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = always_t::static_query<E>();
#endif // defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR blocking_t<I> value()
  {
    return always_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const always_t&, const always_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const always_t&, const always_t&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const always_t&, const possibly_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const always_t&, const possibly_t<I>&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const always_t&, const never_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const always_t&, const never_t<I>&)
  {
    return true;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T always_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <int I>
struct never_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef blocking_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR never_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, never_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, never_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, never_t>::value();
  }

  template <typename E, typename T = decltype(never_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = never_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR blocking_t<I> value()
  {
    return never_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const never_t&, const never_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const never_t&, const never_t&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const never_t&, const possibly_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const never_t&, const possibly_t<I>&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const never_t&, const always_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const never_t&, const always_t<I>&)
  {
    return true;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T never_t<I>::static_query_v;
#endif // defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

} // namespace blocking
} // namespace detail

typedef detail::blocking_t<> blocking_t;

#if defined(ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
constexpr blocking_t blocking;
#else // defined(ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
namespace { static const blocking_t& blocking = blocking_t::instance; }
#endif

} // namespace execution

#if !defined(ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T>
struct is_applicable_property<T, execution::blocking_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::blocking_t::possibly_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::blocking_t::always_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::blocking_t::never_t>
  : execution::is_executor<T>
{
};

#endif // !defined(ASIO_HAS_VARIABLE_TEMPLATES)

namespace traits {

#if !defined(ASIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

template <typename T>
struct query_free_default<T, execution::blocking_t,
  typename enable_if<
    can_query<T, execution::blocking_t::possibly_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::blocking_t::possibly_t>::value));

  typedef execution::blocking_t result_type;
};

template <typename T>
struct query_free_default<T, execution::blocking_t,
  typename enable_if<
    !can_query<T, execution::blocking_t::possibly_t>::value
      && can_query<T, execution::blocking_t::always_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::blocking_t::always_t>::value));

  typedef execution::blocking_t result_type;
};

template <typename T>
struct query_free_default<T, execution::blocking_t,
  typename enable_if<
    !can_query<T, execution::blocking_t::possibly_t>::value
      && !can_query<T, execution::blocking_t::always_t>::value
      && can_query<T, execution::blocking_t::never_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::blocking_t::never_t>::value));

  typedef execution::blocking_t result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  || !defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <typename T>
struct static_query<T, execution::blocking_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T, execution::blocking_t>::is_valid
      && !traits::query_member<T, execution::blocking_t>::is_valid
      && traits::static_query<T, execution::blocking_t::possibly_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::blocking_t::possibly_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T, execution::blocking_t::possibly_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T, execution::blocking_t>::is_valid
      && !traits::query_member<T, execution::blocking_t>::is_valid
      && !traits::static_query<T, execution::blocking_t::possibly_t>::is_valid
      && traits::static_query<T, execution::blocking_t::always_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::blocking_t::always_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T, execution::blocking_t::always_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T, execution::blocking_t>::is_valid
      && !traits::query_member<T, execution::blocking_t>::is_valid
      && !traits::static_query<T, execution::blocking_t::possibly_t>::is_valid
      && !traits::static_query<T, execution::blocking_t::always_t>::is_valid
      && traits::static_query<T, execution::blocking_t::never_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::blocking_t::never_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T, execution::blocking_t::never_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t::possibly_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_t::possibly_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_t::possibly_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_t::possibly_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t::possibly_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
      execution::blocking_t::possibly_t>::is_valid
      && !traits::query_member<T, execution::blocking_t::possibly_t>::is_valid
      && !traits::query_free<T, execution::blocking_t::possibly_t>::is_valid
      && !can_query<T, execution::blocking_t::always_t>::value
      && !can_query<T, execution::blocking_t::never_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef execution::blocking_t::possibly_t result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return result_type();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t::always_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_t::always_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_t::always_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_t::always_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_t::never_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_t::never_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_t::never_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_t::never_t>::value();
  }
};

#endif // !defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   || !defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

#if !defined(ASIO_HAS_DEDUCED_STATIC_REQUIRE_TRAIT)

template <typename T>
struct static_require<T, execution::blocking_t::possibly_t,
  typename enable_if<
    static_query<T, execution::blocking_t::possibly_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::blocking_t::possibly_t>::result_type,
        execution::blocking_t::possibly_t>::value));
};

template <typename T>
struct static_require<T, execution::blocking_t::always_t,
  typename enable_if<
    static_query<T, execution::blocking_t::always_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::blocking_t::always_t>::result_type,
        execution::blocking_t::always_t>::value));
};

template <typename T>
struct static_require<T, execution::blocking_t::never_t,
  typename enable_if<
    static_query<T, execution::blocking_t::never_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::blocking_t::never_t>::result_type,
        execution::blocking_t::never_t>::value));
};

#endif // !defined(ASIO_HAS_DEDUCED_STATIC_REQUIRE_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_BLOCKING_HPP
