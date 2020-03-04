//
// execution/executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_EXECUTOR_HPP
#define ASIO_EXECUTION_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution/execute.hpp"
#include "asio/execution/invocable_archetype.hpp"
#include "asio/traits/equality_comparable.hpp"

#include "asio/detail/push_options.hpp"

#if defined(ASIO_HAS_DEDUCED_EXECUTE_FREE_TRAIT) \
  && defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT) \
  && defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)
# define ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT 1
#endif // defined(ASIO_HAS_DEDUCED_EXECUTE_FREE_TRAIT)
       //   && defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
       //   && defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

namespace asio {
namespace execution {

template <typename T>
struct is_executor :
  integral_constant<bool,
    can_execute<const T, invocable_archetype>::value
#if defined(ASIO_HAS_NOEXCEPT)
      && is_nothrow_copy_constructible<T>::value
      && is_nothrow_destructible<T>::value
#else // defined(ASIO_HAS_NOEXCEPT)
      && is_copy_constructible<T>::value
      && is_destructible<T>::value
#endif // defined(ASIO_HAS_NOEXCEPT)
      && traits::equality_comparable<typename decay<T>::type>::is_valid
      && traits::equality_comparable<typename decay<T>::type>::is_noexcept
  >
{
};

#if defined(ASIO_HAS_CONCEPTS)

template <typename T>
ASIO_CONCEPT executor = is_executor<T>::value;

#define ASIO_EXECUTION_EXECUTOR ::asio::execution::executor

#else // defined(ASIO_HAS_CONCEPTS)

#define ASIO_EXECUTION_EXECUTOR typename

#endif // defined(ASIO_HAS_CONCEPTS)

} // namespace execution
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_EXECUTOR_HPP
