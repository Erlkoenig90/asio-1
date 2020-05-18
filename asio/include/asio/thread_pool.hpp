//
// thread_pool.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_THREAD_POOL_HPP
#define ASIO_THREAD_POOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/scheduler.hpp"
#include "asio/detail/thread_group.hpp"
#include "asio/execution.hpp"
#include "asio/execution_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

/// A simple fixed-size thread pool.
/**
 * The thread pool class is an execution context where functions are permitted
 * to run on one of a fixed number of threads.
 *
 * @par Submitting tasks to the pool
 *
 * To submit functions to the thread pool, use the @ref asio::dispatch,
 * @ref asio::post or @ref asio::defer free functions.
 *
 * For example:
 *
 * @code void my_task()
 * {
 *   ...
 * }
 *
 * ...
 *
 * // Launch the pool with four threads.
 * asio::thread_pool pool(4);
 *
 * // Submit a function to the pool.
 * asio::post(pool, my_task);
 *
 * // Submit a lambda object to the pool.
 * asio::post(pool,
 *     []()
 *     {
 *       ...
 *     });
 *
 * // Wait for all tasks in the pool to complete.
 * pool.join(); @endcode
 */
class thread_pool
  : public execution_context
{
public:
  template <typename Blocking, typename Relationship,
      typename OutstandingWork, typename Allocator>
  class basic_executor_type;

  typedef basic_executor_type<
      execution::blocking_t::possibly_t,
      execution::relationship_t::fork_t,
      execution::outstanding_work_t::untracked_t,
      std::allocator<void> > executor_type;

#if !defined(ASIO_STANDARD_EXECUTORS_ONLY)
  /// Constructs a pool with an automatically determined number of threads.
  ASIO_DECL thread_pool();
#endif // !defined(ASIO_STANDARD_EXECUTORS_ONLY)

  /// Constructs a pool with a specified number of threads.
  ASIO_DECL thread_pool(std::size_t num_threads);

  /// Destructor.
  /**
   * Automatically stops and joins the pool, if not explicitly done beforehand.
   */
  ASIO_DECL ~thread_pool();

  /// Obtains the executor associated with the pool.
  executor_type get_executor() ASIO_NOEXCEPT;

  /// Obtains the executor associated with the pool.
  executor_type executor() ASIO_NOEXCEPT;

  /// Stops the threads.
  /**
   * This function stops the threads as soon as possible. As a result of calling
   * @c stop(), pending function objects may be never be invoked.
   */
  ASIO_DECL void stop();

  /// Attaches the current thread to the pool.
  /**
   * This function attaches the current thread to the pool so that it may be
   * used for executing submitted function objects. Blocks the calling thread
   * until the pool is stopped or joined and has no outstanding work.
   */
  ASIO_DECL void attach();

  /// Joins the threads.
  /**
   * This function blocks until the threads in the pool have completed. If @c
   * stop() is not called prior to @c join(), the @c join() call will wait
   * until the pool has no more outstanding work.
   */
  ASIO_DECL void join();

  /// Waits for threads to complete.
  /**
   * This function blocks until the threads in the pool have completed. If @c
   * stop() is not called prior to @c wait(), the @c wait() call will wait
   * until the pool has no more outstanding work.
   */
  ASIO_DECL void wait();

private:
  struct thread_function;

  // Helper function to create the underlying scheduler.
  ASIO_DECL detail::scheduler& add_scheduler(detail::scheduler* s);

  // The underlying scheduler.
  detail::scheduler& scheduler_;

  // The threads in the pool.
  detail::thread_group threads_;

  // The number of threads in the pool at construction.
  std::size_t num_threads_;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
class thread_pool::basic_executor_type
{
public:
  /// Copy construtor.
  basic_executor_type(const basic_executor_type& other) ASIO_NOEXCEPT;

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move construtor.
  basic_executor_type(basic_executor_type&& other) ASIO_NOEXCEPT;
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destructor.
  ~basic_executor_type();

  /// Assignment operator.
  basic_executor_type& operator=(
      const basic_executor_type& other) ASIO_NOEXCEPT;

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move assignment operator.
  basic_executor_type& operator=(
      basic_executor_type&& other) ASIO_NOEXCEPT;
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Obtain an executor with the @c blocking.possibly property.
  basic_executor_type<execution::blocking_t::possibly_t,
      Relationship, OutstandingWork, Allocator>
  require(execution::blocking_t::possibly_t) const
  {
    return basic_executor_type<execution::blocking_t::possibly_t, Relationship,
        OutstandingWork, Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the @c blocking.always property.
  basic_executor_type<execution::blocking_t::always_t,
      Relationship, OutstandingWork, Allocator>
  require(execution::blocking_t::always_t) const
  {
    return basic_executor_type<execution::blocking_t::always_t, Relationship,
        OutstandingWork, Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the @c blocking.never property.
  basic_executor_type<execution::blocking_t::never_t,
      Relationship, OutstandingWork, Allocator>
  require(execution::blocking_t::never_t) const
  {
    return basic_executor_type<execution::blocking_t::never_t, Relationship,
        OutstandingWork, Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the @c relationship.fork property.
  basic_executor_type<Blocking, execution::relationship_t::fork_t,
      OutstandingWork, Allocator>
  require(execution::relationship_t::fork_t) const
  {
    return basic_executor_type<Blocking,
        execution::relationship_t::fork_t, OutstandingWork,
        Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the @c relationship.continuation property.
  basic_executor_type<Blocking, execution::relationship_t::continuation_t,
      OutstandingWork, Allocator>
  require(execution::relationship_t::continuation_t) const
  {
    return basic_executor_type<Blocking,
        execution::relationship_t::continuation_t, OutstandingWork,
        Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the @c outstanding_work.untracked property.
  basic_executor_type<Blocking, Relationship,
      execution::outstanding_work_t::untracked_t, Allocator>
  require(execution::outstanding_work_t::untracked_t) const
  {
    return basic_executor_type<Blocking, Relationship,
        execution::outstanding_work_t::untracked_t,
        Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the @c outstanding_work.tracked property.
  basic_executor_type<Blocking, Relationship,
      execution::outstanding_work_t::tracked_t, Allocator>
  require(execution::outstanding_work_t::tracked_t) const
  {
    return basic_executor_type<Blocking, Relationship,
        execution::outstanding_work_t::tracked_t,
        Allocator>(pool_, allocator_);
  }

  /// Obtain an executor with the specified @c allocator property.
  template <typename OtherAllocator>
  basic_executor_type<Blocking, Relationship, OutstandingWork, OtherAllocator>
  require(execution::allocator_t<OtherAllocator> a) const
  {
    return basic_executor_type<Blocking, Relationship,
        OutstandingWork, OtherAllocator>(pool_, a.value());
  }

  /// Obtain an executor with the default @c allocator property.
  basic_executor_type<Blocking, Relationship,
      OutstandingWork, std::allocator<void> >
  require(execution::allocator_t<void>) const
  {
    return basic_executor_type<Blocking, Relationship,
        OutstandingWork, std::allocator<void> >(
          pool_, std::allocator<void>());
  }

  /// Query the current value of the @c bulk_guarantee property.
  static ASIO_CONSTEXPR execution::bulk_guarantee_t query(
      execution::bulk_guarantee_t) ASIO_NOEXCEPT
  {
    return execution::bulk_guarantee.parallel;
  }

  /// Query the current value of the @c mapping property.
  static ASIO_CONSTEXPR execution::mapping_t query(
      execution::mapping_t) ASIO_NOEXCEPT
  {
    return execution::mapping.thread;
  }

  /// Query the current value of the @c blocking property.
  ASIO_CONSTEXPR execution::blocking_t query(
      execution::blocking_t) const ASIO_NOEXCEPT
  {
    return Blocking();
  }

  /// Query the current value of the @c relationship property.
  ASIO_CONSTEXPR execution::relationship_t query(
      execution::relationship_t) const ASIO_NOEXCEPT
  {
    return Relationship();
  }

  /// Query the current value of the @c outstanding_work property.
  ASIO_CONSTEXPR execution::outstanding_work_t query(
      execution::outstanding_work_t) const ASIO_NOEXCEPT
  {
    return OutstandingWork();
  }

  /// Query the current value of the @c context property.
  thread_pool& query(execution::context_t) const ASIO_NOEXCEPT
  {
    return *pool_;
  }

  /// Query the current value of the @c allocator property.
  template <typename OtherAllocator>
  ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<OtherAllocator>) const ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the current value of the @c allocator property.
  ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<void>) const ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the occupancy (recommended number of work items) for the pool.
  std::size_t query(execution::occupancy_t) const ASIO_NOEXCEPT
  {
    return (pool_->num_threads_ < 1 ? 1 : pool_->num_threads_) * 2;
  }

  /// Determine whether the thread pool is running in the current thread.
  /**
   * @return @c true if the current thread is running the thread pool. Otherwise
   * returns @c false.
   */
  bool running_in_this_thread() const ASIO_NOEXCEPT;

  /// Compare two executors for equality.
  /**
   * Two executors are equal if they refer to the same underlying thread pool.
   */
  friend bool operator==(const basic_executor_type& a,
      const basic_executor_type& b) ASIO_NOEXCEPT
  {
    return a.pool_ == b.pool_;
  }

  /// Compare two executors for inequality.
  /**
   * Two executors are equal if they refer to the same underlying thread pool.
   */
  friend bool operator!=(const basic_executor_type& a,
      const basic_executor_type& b) ASIO_NOEXCEPT
  {
    return a.pool_ != b.pool_;
  }

  /// Execution function.
  template <typename Function>
  void execute(ASIO_MOVE_ARG(Function) f) const
  {
    this->do_execute(ASIO_MOVE_CAST(Function)(f), Blocking());
  }

#if !defined(ASIO_STANDARD_EXECUTORS_ONLY)
  /// Obtain the underlying execution context.
  thread_pool& context() const ASIO_NOEXCEPT;

  /// Inform the thread pool that it has some outstanding work to do.
  /**
   * This function is used to inform the thread pool that some work has begun.
   * This ensures that the thread pool's join() function will not return while
   * the work is underway.
   */
  void on_work_started() const ASIO_NOEXCEPT;

  /// Inform the thread pool that some work is no longer outstanding.
  /**
   * This function is used to inform the thread pool that some work has
   * finished. Once the count of unfinished work reaches zero, the thread
   * pool's join() function is permitted to exit.
   */
  void on_work_finished() const ASIO_NOEXCEPT;

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. If the current thread belongs to the pool, @c dispatch() executes
   * the function before returning. Otherwise, the function will be scheduled
   * to run on the thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void dispatch(ASIO_MOVE_ARG(Function) f,
      const OtherAllocator& a) const;

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. The function object will never be executed inside @c post().
   * Instead, it will be scheduled to run on the thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void post(ASIO_MOVE_ARG(Function) f,
      const OtherAllocator& a) const;

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. The function object will never be executed inside @c defer().
   * Instead, it will be scheduled to run on the thread pool.
   *
   * If the current thread belongs to the thread pool, @c defer() will delay
   * scheduling the function object until the current thread returns control to
   * the pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void defer(ASIO_MOVE_ARG(Function) f,
      const OtherAllocator& a) const;
#endif // !defined(ASIO_STANDARD_EXECUTORS_ONLY)

private:
  friend class thread_pool;
  template <typename, typename, typename, typename>
    friend class basic_executor_type;

  // Constructor used by thread_pool::get_executor().
  explicit basic_executor_type(thread_pool& p) ASIO_NOEXCEPT;

  // Constructor used by require().
  basic_executor_type(thread_pool* p, const Allocator& a) ASIO_NOEXCEPT;

  /// Execution helper implementation for the possibly blocking property.
  template <typename Function>
  void do_execute(ASIO_MOVE_ARG(Function) f,
      execution::blocking_t::possibly_t) const;

  /// Execution helper implementation for the always blocking property.
  template <typename Function>
  void do_execute(ASIO_MOVE_ARG(Function) f,
      execution::blocking_t::always_t) const;

  /// Execution helper implementation for the never blocking property.
  template <typename Function>
  void do_execute(ASIO_MOVE_ARG(Function) f,
      execution::blocking_t::never_t) const;

  // The underlying thread pool.
  thread_pool* pool_;

  // The allocator used for execution functions.
  Allocator allocator_;
};

namespace execution {

#if !defined(ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct is_executor<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>
    > : true_type
{
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

} // namespace execution
namespace traits {

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename Function>
struct execute_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    Function
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::blocking_t::possibly_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      asio::execution::blocking_t::possibly_t, Relationship,
      OutstandingWork, Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::blocking_t::always_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      asio::execution::blocking_t::always_t, Relationship,
      OutstandingWork, Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::blocking_t::never_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      asio::execution::blocking_t::never_t, Relationship,
      OutstandingWork, Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::relationship_t::fork_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      Blocking, asio::execution::relationship_t::fork_t,
      OutstandingWork, Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::relationship_t::continuation_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      Blocking, asio::execution::relationship_t::continuation_t,
      OutstandingWork, Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::outstanding_work_t::untracked_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      Blocking, Relationship,
      asio::execution::outstanding_work_t::untracked_t,
      Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::outstanding_work_t::tracked_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      Blocking, Relationship,
      asio::execution::outstanding_work_t::tracked_t,
      Allocator> result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::allocator_t<void>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork,
      std::allocator<void> > result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename OtherAllocator>
struct require_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::allocator_t<OtherAllocator>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork,
      OtherAllocator> result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename Property>
struct query_static_constexpr_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::bulk_guarantee_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::bulk_guarantee_t::parallel_t result_type;

  static ASIO_CONSTEXPR result_type value() ASIO_NOEXCEPT
  {
    return result_type();
  }
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename Property>
struct query_static_constexpr_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::mapping_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::mapping_t::thread_t result_type;

  static ASIO_CONSTEXPR result_type value() ASIO_NOEXCEPT
  {
    return result_type();
  }
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename Property>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::blocking_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::blocking_t result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename Property>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::relationship_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::relationship_t result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator, typename Property>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::outstanding_work_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::outstanding_work_t result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::occupancy_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef std::size_t result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::context_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::thread_pool& result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::allocator_t<void>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

template <typename Blocking, typename Relationship,
    typename OutstandingWork, typename Allocator>
struct query_member<
    asio::thread_pool::basic_executor_type<
      Blocking, Relationship, OutstandingWork, Allocator>,
    asio::execution::allocator_t<Allocator>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/impl/thread_pool.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/impl/thread_pool.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_THREAD_POOL_HPP
