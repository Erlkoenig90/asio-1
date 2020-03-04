//
// execution/execute.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_EXECUTE_HPP
#define ASIO_EXECUTION_EXECUTE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/traits/execute_member.hpp"
#include "asio/traits/execute_free.hpp"

namespace asio_execution_execute_fn {

using asio::decay;
using asio::declval;
using asio::enable_if;
using asio::traits::execute_free;
using asio::traits::execute_member;

enum overload_type
{
  call_member,
  call_free,
  ill_formed
};

template <typename T, typename Properties, typename = void>
struct call_traits
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = ill_formed);
};

template <typename T, typename F>
struct call_traits<T, void(F),
  typename enable_if<
    (
      execute_member<T, F>::is_valid
    )
  >::type> :
  execute_member<T, F>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = call_member);
};

template <typename T, typename F>
struct call_traits<T, void(F),
  typename enable_if<
    (
      !execute_member<T, F>::is_valid
      &&
      execute_free<T, F>::is_valid
    )
  >::type> :
  execute_free<T, F>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = call_free);
};

struct impl
{
  template <typename T, typename F>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(F)>::overload == call_member,
    typename call_traits<T, void(F)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(F) p) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(F)>::is_noexcept))
  {
    return ASIO_MOVE_CAST(T)(t).execute(ASIO_MOVE_CAST(F)(p));
  }

  template <typename T, typename F>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(F)>::overload == call_free,
    typename call_traits<T, void(F)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(F) p) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(F)>::is_noexcept))
  {
    return execute(ASIO_MOVE_CAST(T)(t), ASIO_MOVE_CAST(F)(p));
  }
};

template <typename T = impl>
struct static_instance
{
  static const T instance;
};

template <typename T>
const T static_instance<T>::instance = {};

} // namespace asio_execution_execute_fn
namespace asio {
namespace execution {
namespace {

static ASIO_CONSTEXPR const asio_execution_execute_fn::impl&
  execute = asio_execution_execute_fn::static_instance<>::instance;

} // namespace

template <typename T, typename F>
struct can_execute :
  integral_constant<bool,
    asio_execution_execute_fn::call_traits<T, void(F)>::overload !=
      asio_execution_execute_fn::ill_formed>
{
};

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T, typename F>
constexpr bool can_execute_v = can_execute<T, F>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

} // namespace execution
} // namespace asio

#endif // ASIO_EXECUTION_EXECUTE_HPP
