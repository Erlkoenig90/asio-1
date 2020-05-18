//
// impl/thread_pool.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_THREAD_POOL_HPP
#define ASIO_IMPL_THREAD_POOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/blocking_executor_op.hpp"
#include "asio/detail/executor_op.hpp"
#include "asio/detail/fenced_block.hpp"
#include "asio/detail/non_const_lvalue.hpp"
#include "asio/detail/recycling_allocator.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

inline thread_pool::executor_type
thread_pool::get_executor() ASIO_NOEXCEPT
{
  return executor_type(*this);
}

inline thread_pool::executor_type
thread_pool::executor() ASIO_NOEXCEPT
{
  return executor_type(*this);
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline thread_pool::basic_executor_type<Blocking, Relationship,
    OutstandingWork, Allocator>::basic_executor_type(
    thread_pool& p) ASIO_NOEXCEPT
  : pool_(&p),
    allocator_()
{
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
    pool_->scheduler_.work_started();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline thread_pool::basic_executor_type<Blocking, Relationship,
    OutstandingWork, Allocator>::basic_executor_type(
    thread_pool* p, const Allocator& a) ASIO_NOEXCEPT
  : pool_(p),
    allocator_(a)
{
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
    pool_->scheduler_.work_started();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline thread_pool::basic_executor_type<Blocking, Relationship,
    OutstandingWork, Allocator>::basic_executor_type(
    const basic_executor_type& other) ASIO_NOEXCEPT
  : pool_(other.pool_),
    allocator_(other.allocator_)
{
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
    pool_->scheduler_.work_started();
}

#if defined(ASIO_HAS_MOVE)
template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline thread_pool::basic_executor_type<Blocking, Relationship,
    OutstandingWork, Allocator>::basic_executor_type(
    basic_executor_type&& other) ASIO_NOEXCEPT
  : pool_(other.pool_),
    allocator_(ASIO_MOVE_CAST(Allocator)(other.allocator_))
{
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
    pool_->scheduler_.work_started();
}
#endif // defined(ASIO_HAS_MOVE)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline thread_pool::basic_executor_type<Blocking, Relationship,
    OutstandingWork, Allocator>::~basic_executor_type()
{
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
    pool_->scheduler_.work_finished();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
thread_pool::basic_executor_type<Blocking,
    Relationship, OutstandingWork, Allocator>&
thread_pool::basic_executor_type<Blocking, Relationship, OutstandingWork,
    Allocator>::operator=(const basic_executor_type& other) ASIO_NOEXCEPT
{
  thread_pool* old_pool = pool_;
  pool_ = other.pool_;
  allocator_ = other.allocator_;
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
  {
    pool_->scheduler_.work_started();
    old_pool->scheduler_.work_finished();
  }
  return *this;
}

#if defined(ASIO_HAS_MOVE)
template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
thread_pool::basic_executor_type<Blocking,
    Relationship, OutstandingWork, Allocator>&
thread_pool::basic_executor_type<Blocking, Relationship, OutstandingWork,
    Allocator>::operator=(basic_executor_type&& other) ASIO_NOEXCEPT
{
  thread_pool* old_pool = pool_;
  pool_ = other.pool_;
  allocator_ = std::move(other.allocator_);
  if (is_same<OutstandingWork, execution::outstanding_work_t::tracked_t>::value)
  {
    pool_->scheduler_.work_started();
    old_pool->scheduler_.work_finished();
  }
  return *this;
}
#endif // defined(ASIO_HAS_MOVE)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline bool thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::running_in_this_thread() const ASIO_NOEXCEPT
{
  return pool_->scheduler_.can_dispatch();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
template <typename Function>
void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::do_execute(ASIO_MOVE_ARG(Function) f,
    execution::blocking_t::possibly_t) const
{
  // Invoke immediately if we are already inside the thread pool.
  if (pool_->scheduler_.can_dispatch())
  {
    // Obtain a non-const instance of the function.
    detail::non_const_lvalue<Function> f2(f);

    detail::fenced_block b(detail::fenced_block::full);
    asio_handler_invoke_helpers::invoke(f2.value, f2.value);
    return;
  }

  this->do_execute(ASIO_MOVE_CAST(Function)(f),
      execution::blocking_t::never_t());
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
template <typename Function>
void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::do_execute(ASIO_MOVE_ARG(Function) f,
    execution::blocking_t::always_t) const
{
  // Obtain a non-const instance of the function.
  detail::non_const_lvalue<Function> f2(f);

  // Invoke immediately if we are already inside the thread pool.
  if (pool_->scheduler_.can_dispatch())
  {
    detail::fenced_block b(detail::fenced_block::full);
    asio_handler_invoke_helpers::invoke(f2.value, f2.value);
    return;
  }

  // Construct an operation to wrap the function.
  typedef typename decay<Function>::type function_type;
  detail::blocking_executor_op<function_type> op(f2.value);

  ASIO_HANDLER_CREATION((*pool_, op,
        "thread_pool", pool_, 0, "execute(blk=always)"));

  pool_->scheduler_.post_immediate_completion(&op, false);
  op.wait();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
template <typename Function>
void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::do_execute(ASIO_MOVE_ARG(Function) f,
    execution::blocking_t::never_t) const
{
  // Allocate and construct an operation to wrap the function.
  typedef typename decay<Function>::type function_type;
  typedef detail::executor_op<function_type, Allocator> op;
  typename op::ptr p = { detail::addressof(allocator_),
      op::ptr::allocate(allocator_), 0 };
  p.p = new (p.v) op(ASIO_MOVE_CAST(Function)(f), allocator_);

  if (is_same<Relationship, execution::relationship_t::continuation_t>::value)
  {
    ASIO_HANDLER_CREATION((*pool_, *p.p,
          "thread_pool", pool_, 0, "execute(blk=never,rel=cont)"));
  }
  else
  {
    ASIO_HANDLER_CREATION((*pool_, *p.p,
          "thread_pool", pool_, 0, "execute(blk=never,rel=fork)"));
  }

  pool_->scheduler_.post_immediate_completion(p.p,
      is_same<Relationship, execution::relationship_t::continuation_t>::value);
  p.v = p.p = 0;
}

#if !defined(ASIO_STANDARD_EXECUTORS_ONLY)
template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline thread_pool& thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::context() const ASIO_NOEXCEPT
{
  return *pool_;
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::on_work_started() const ASIO_NOEXCEPT
{
  pool_->scheduler_.work_started();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
inline void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork,
    Allocator>::on_work_finished() const ASIO_NOEXCEPT
{
  pool_->scheduler_.work_finished();
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
template <typename Function, typename OtherAllocator>
void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork, Allocator>::dispatch(
    ASIO_MOVE_ARG(Function) f, const OtherAllocator& a) const
{
  typedef typename decay<Function>::type function_type;

  // Invoke immediately if we are already inside the thread pool.
  if (pool_->scheduler_.can_dispatch())
  {
    // Make a local, non-const copy of the function.
    function_type tmp(ASIO_MOVE_CAST(Function)(f));

    detail::fenced_block b(detail::fenced_block::full);
    asio_handler_invoke_helpers::invoke(tmp, tmp);
    return;
  }

  // Allocate and construct an operation to wrap the function.
  typedef detail::executor_op<function_type, OtherAllocator> op;
  typename op::ptr p = { detail::addressof(a), op::ptr::allocate(a), 0 };
  p.p = new (p.v) op(ASIO_MOVE_CAST(Function)(f), a);

  ASIO_HANDLER_CREATION((*pool_, *p.p,
        "thread_pool", pool_, 0, "dispatch"));

  pool_->scheduler_.post_immediate_completion(p.p, false);
  p.v = p.p = 0;
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
template <typename Function, typename OtherAllocator>
void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork, Allocator>::post(
    ASIO_MOVE_ARG(Function) f, const OtherAllocator& a) const
{
  typedef typename decay<Function>::type function_type;

  // Allocate and construct an operation to wrap the function.
  typedef detail::executor_op<function_type, OtherAllocator> op;
  typename op::ptr p = { detail::addressof(a), op::ptr::allocate(a), 0 };
  p.p = new (p.v) op(ASIO_MOVE_CAST(Function)(f), a);

  ASIO_HANDLER_CREATION((*pool_, *p.p,
        "thread_pool", pool_, 0, "post"));

  pool_->scheduler_.post_immediate_completion(p.p, false);
  p.v = p.p = 0;
}

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
template <typename Function, typename OtherAllocator>
void thread_pool::basic_executor_type<
    Blocking, Relationship, OutstandingWork, Allocator>::defer(
    ASIO_MOVE_ARG(Function) f, const OtherAllocator& a) const
{
  typedef typename decay<Function>::type function_type;

  // Allocate and construct an operation to wrap the function.
  typedef detail::executor_op<function_type, OtherAllocator> op;
  typename op::ptr p = { detail::addressof(a), op::ptr::allocate(a), 0 };
  p.p = new (p.v) op(ASIO_MOVE_CAST(Function)(f), a);

  ASIO_HANDLER_CREATION((*pool_, *p.p,
        "thread_pool", pool_, 0, "defer"));

  pool_->scheduler_.post_immediate_completion(p.p, true);
  p.v = p.p = 0;
}
#endif // !defined(ASIO_STANDARD_EXECUTORS_ONLY)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_THREAD_POOL_HPP
