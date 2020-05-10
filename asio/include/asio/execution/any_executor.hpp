//
// execution/any_executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_ANY_EXECUTOR_HPP
#define ASIO_EXECUTION_ANY_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include <new>
#include <typeinfo>

#include "asio/detail/executor_function.hpp"
#include "asio/detail/memory.hpp"
#include "asio/detail/non_const_lvalue.hpp"
#include "asio/detail/scoped_ptr.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/detail/variadic_templates.hpp"
#include "asio/execution/bad_executor.hpp"
#include "asio/execution/blocking.hpp"
#include "asio/execution/executor.hpp"
#include "asio/prefer.hpp"
#include "asio/query.hpp"
#include "asio/require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace execution {

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename... SupportableProperties>
class any_executor;

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename = void, typename = void, typename = void,
    typename = void, typename = void, typename = void>
class any_executor;

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

namespace detail {

// Trait used to detect whether an any_executor instantiation has a subset of
// the properties of another.

template <typename T, typename U>
struct is_subset : false_type {};

template <>
struct is_subset<any_executor<>, any_executor<> > : true_type {};

template <typename U0>
struct is_subset<any_executor<>, any_executor<U0> > : true_type {};

template <typename T0>
struct is_subset<any_executor<T0>, any_executor<> > : false_type {};

template <typename T0>
struct is_subset<any_executor<T0>, any_executor<T0> > : true_type {};

template <typename T0, typename U0>
struct is_subset<any_executor<T0>, any_executor<U0> > : false_type {};

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename U0, typename... UN>
struct is_subset<any_executor<>, any_executor<U0, UN...> > : true_type {};

template <typename T0, typename... UN>
struct is_subset<any_executor<T0>, any_executor<T0, UN...> > : true_type {};

template <typename T0, typename U0, typename... UN>
struct is_subset<any_executor<T0>, any_executor<U0, UN...> > :
  is_subset<any_executor<T0>, any_executor<UN...> > {};

template <typename T0, typename... TN>
struct is_subset<any_executor<T0, TN...>, any_executor<> > : false_type {};

template <typename T0, typename... TN>
struct is_subset<any_executor<T0, TN...>, any_executor<T0> > :
  is_subset<any_executor<TN...>, any_executor<T0> > {};

template <typename T0, typename... TN, typename U0>
struct is_subset<any_executor<T0, TN...>, any_executor<U0> > : false_type {};

template <typename T0, typename... TN, typename... UN>
struct is_subset<any_executor<T0, TN...>, any_executor<T0, UN...> > :
  is_subset<any_executor<TN...>, any_executor<T0, UN...> > {};

template <typename T0, typename... TN, typename U0, typename... UN>
struct is_subset<any_executor<T0, TN...>, any_executor<U0, UN...> > :
  integral_constant<bool,
    is_subset<any_executor<T0>, any_executor<U0, UN...> >::value
      && is_subset<any_executor<TN...>, any_executor<U0, UN...> >::value> {};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename U0, typename U1, typename U2, typename U3, typename U4>
struct is_subset<any_executor<>, any_executor<U0, U1, U2, U3, U4> > :
  true_type {};

template <typename T0, typename U1, typename U2, typename U3, typename U4>
struct is_subset<any_executor<T0>, any_executor<T0, U1, U2, U3, U4> > :
  true_type {};

template <typename T0,
    typename U0, typename U1, typename U2, typename U3, typename U4>
struct is_subset<any_executor<T0>, any_executor<U0, U1, U2, U3, U4> > :
  is_subset<any_executor<T0>, any_executor<U1, U2, U3, U4> > {};

template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct is_subset<any_executor<T0, T1, T2, T3, T4>, any_executor<> > :
  false_type {};

template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct is_subset<any_executor<T0, T1, T2, T3, T4>, any_executor<T0> > :
  is_subset<any_executor<T1, T2, T3, T4>, any_executor<T0> > {};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
    typename U0>
struct is_subset<any_executor<T0, T1, T2, T3, T4>, any_executor<U0> > :
  false_type {};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
    typename U1, typename U2, typename U3, typename U4>
struct is_subset<any_executor<T0, T1, T2, T3, T4>,
  any_executor<T0, U1, U2, U3, U4> > :
    is_subset<any_executor<T1, T2, T3, T4>,
      any_executor<T0, U1, U2, U3, U4> > {};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
    typename U0, typename U1, typename U2, typename U3, typename U4>
struct is_subset<any_executor<T0, T1, T2, T3, T4>,
  any_executor<U0, U1, U2, U3, U4> > :
    integral_constant<bool,
      is_subset<any_executor<T0>, any_executor<U0, U1, U2, U3, U4> >::value
        && is_subset<any_executor<T1, T2, T3, T4>,
          any_executor<U0, U1, U2, U3, U4> >::value> {};

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

// Base class tag types used to detect whether a particular any_executor
// instantiation supports a given property.

template <typename Prop> class any_executor_prop_query_tag {};
template <typename Prop> class any_executor_prop_require_tag {};
template <typename Prop> class any_executor_prop_prefer_tag {};

// Implementations of any_executor operations query, require and prefer for
// each of the supportable properties.

template <typename Derived, std::size_t I, typename Prop,
    typename ResultType = typename Prop::polymorphic_query_result_type>
class any_executor_prop_query :
  public any_executor_prop_query_tag<Prop>
{
protected:
  template <typename T>
  struct is_valid_target : can_query<typename add_const<T>::type, Prop> {};

  typename Prop::polymorphic_query_result_type query(const Prop& p) const
  {
    const Derived& ex = *static_cast<const Derived*>(this);
    return *asio::detail::scoped_ptr<
      typename Prop::polymorphic_query_result_type>(
        static_cast<typename Prop::polymorphic_query_result_type*>(
          ex.prop_fns_[I].query(ex.object_fns_->target(ex), &p)));
  }
};

template <typename Derived, std::size_t I, typename Prop>
class any_executor_prop_query<Derived, I, Prop, void> :
  public any_executor_prop_query_tag<Prop>
{
protected:
  template <typename T>
  struct is_valid_target : can_query<typename add_const<T>::type, Prop> {};

  void query(const Prop& p) const
  {
    const Derived& ex = *static_cast<const Derived*>(this);
    ex.prop_fns_[I].query(ex.object_fns_->target(ex), &p);
  }
};

template <typename Derived, std::size_t I, typename Prop, typename = void>
class any_executor_prop_require
{
protected:
  template <typename T>
  struct is_valid_target : true_type{};

  template <typename RequestedProp>
  typename enable_if<
    conditional<true, false_type, RequestedProp>::type::value
  >::type require(const RequestedProp&) const;
};

template <typename Derived, std::size_t I, typename Prop>
class any_executor_prop_require<Derived, I, Prop,
    typename enable_if<
      Prop::is_requirable
    >::type> :
  public any_executor_prop_require_tag<Prop>
{
protected:
  template <typename T>
  struct is_valid_target : can_require<typename add_const<T>::type, Prop> {};

  Derived require(const Prop& p) const
  {
    const Derived& ex = *static_cast<const Derived*>(this);
    return ex.prop_fns_[I].require(ex.object_fns_->target(ex), &p);
  }
};

template <typename Derived, std::size_t I, typename Prop, typename = void>
class any_executor_prop_prefer
{
protected:
  template <typename T>
  struct is_valid_target : true_type{};
};

template <typename Derived, std::size_t I, typename Prop>
class any_executor_prop_prefer<Derived, I, Prop,
    typename enable_if<
      Prop::is_preferable
    >::type> :
  public any_executor_prop_prefer_tag<Prop>
{
protected:
  template <typename T>
  struct is_valid_target : can_prefer<typename add_const<T>::type, Prop> {};

  friend Derived prefer(const Derived& ex, const Prop& p)
  {
    return ex.prop_fns_[I].prefer(ex.object_fns_->target(ex), &p);
  }
};

template <typename Derived, std::size_t I, typename Props>
class any_executor_props_base;

template <typename Derived, std::size_t I, typename Prop>
class any_executor_props_base<Derived, I, void(Prop)> :
  public any_executor_prop_query<Derived, I, Prop>,
  public any_executor_prop_require<Derived, I, Prop>,
  public any_executor_prop_prefer<Derived, I, Prop>
{
protected:
  template <typename T>
  struct is_valid_target : integral_constant<bool,
    any_executor_prop_query<
        Derived, I, Prop>::template is_valid_target<T>::value
      && any_executor_prop_require<
          Derived, I, Prop>::template is_valid_target<T>::value
      && any_executor_prop_prefer<
          Derived, I, Prop>::template is_valid_target<T>::value>
  {
  };

  using any_executor_prop_query<Derived, I, Prop>::query;
  using any_executor_prop_require<Derived, I, Prop>::require;
};

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename Derived, std::size_t I, typename Head, typename... Tail>
class any_executor_props_base<Derived, I, void(Head, Tail...)> :
  public any_executor_prop_query<Derived, I, Head>,
  public any_executor_prop_require<Derived, I, Head>,
  public any_executor_prop_prefer<Derived, I, Head>,
  public any_executor_props_base<Derived, I + 1, void(Tail...)>
{
protected:
  template <typename T>
  struct is_valid_target : integral_constant<bool,
    any_executor_prop_query<
        Derived, I, Head>::template is_valid_target<T>::value
      && any_executor_prop_require<
          Derived, I, Head>::template is_valid_target<T>::value
      && any_executor_prop_prefer<
          Derived, I, Head>::template is_valid_target<T>::value
      && any_executor_props_base<
          Derived, I + 1, void(Tail...)>::template is_valid_target<T>::value>
  {
  };

  using any_executor_prop_query<Derived, I, Head>::query;
  using any_executor_prop_require<Derived, I, Head>::require;
  using any_executor_props_base<Derived, I + 1, void(Tail...)>::query;
  using any_executor_props_base<Derived, I + 1, void(Tail...)>::require;
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#define ASIO_PRIVATE_ANY_EXECUTOR_PROPS_BASE_DEF(n) \
  template <typename Derived, std::size_t I, \
      typename Head, ASIO_VARIADIC_TPARAMS(n)> \
  class any_executor_props_base<Derived, I, \
      void(Head, ASIO_VARIADIC_TARGS(n))> : \
    public any_executor_prop_query<Derived, I, Head>, \
    public any_executor_prop_require<Derived, I, Head>, \
    public any_executor_prop_prefer<Derived, I, Head>, \
    public any_executor_props_base<Derived, \
        I + 1, void(ASIO_VARIADIC_TARGS(n))> \
  { \
  protected: \
    template <typename T> \
    struct is_valid_target : integral_constant<bool, \
      any_executor_prop_query< \
          Derived, I, Head>::template is_valid_target<T>::value \
        && any_executor_prop_require< \
            Derived, I, Head>::template is_valid_target<T>::value \
        && any_executor_prop_prefer< \
            Derived, I, Head>::template is_valid_target<T>::value \
        && any_executor_props_base< \
            Derived, I + 1, void(ASIO_VARIADIC_TARGS(n))>::template \
              is_valid_target<T>::value> \
    { \
    }; \
    \
    using any_executor_prop_query<Derived, I, Head>::query; \
    using any_executor_prop_require<Derived, I, Head>::require; \
    using any_executor_props_base<Derived, \
        I + 1, void(ASIO_VARIADIC_TARGS(n))>::query; \
    using any_executor_props_base<Derived, \
        I + 1, void(ASIO_VARIADIC_TARGS(n))>::require; \
  }; \
  /**/
ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_ANY_EXECUTOR_PROPS_BASE_DEF)
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROPS_BASE_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

class any_executor_base
{
public:
  any_executor_base() ASIO_NOEXCEPT
    : object_fns_(object_fns_table<void>()),
      target_(0),
      target_fns_(target_fns_table<void>()),
      blocking_()
  {
  }

  template <ASIO_EXECUTION_EXECUTOR Executor>
  any_executor_base(Executor ex, false_type)
    : target_fns_(target_fns_table<Executor>()),
      blocking_(
          any_executor_base::query_blocking(
            ex, can_query<const Executor&, const execution::blocking_t&>()))
  {
    any_executor_base::construct_object(ex,
        integral_constant<bool,
          sizeof(Executor) <= sizeof(object_type)
            && alignment_of<Executor>::value <= alignment_of<object_type>::value
        >());
  }

  template <ASIO_EXECUTION_EXECUTOR Executor>
  any_executor_base(Executor other, true_type)
    : object_fns_(object_fns_table<asio::detail::shared_ptr<void> >()),
      target_fns_(other.target_fns_),
      blocking_(other.blocking_)
  {
    asio::detail::shared_ptr<Executor> p =
      asio::detail::make_shared<Executor>(
          ASIO_MOVE_CAST(Executor)(other));
    target_ = p->template target<void>();
    new (&object_) asio::detail::shared_ptr<void>(
        ASIO_MOVE_CAST(asio::detail::shared_ptr<Executor>)(p));
  }

  any_executor_base(const any_executor_base& other) ASIO_NOEXCEPT
    : object_fns_(other.object_fns_),
      target_fns_(other.target_fns_),
      blocking_(other.blocking_)
  {
    object_fns_->copy(*this, other);
  }

  ~any_executor_base()
  {
    object_fns_->destroy(*this);
  }

  any_executor_base& operator=(
      const any_executor_base& other) ASIO_NOEXCEPT
  {
    if (this != &other)
    {
      object_fns_->destroy(*this);
      object_fns_ = other.object_fns_;
      target_fns_ = other.target_fns_;
      blocking_ = other.blocking_;
      object_fns_->copy(*this, other);
    }
    return *this;
  }

#if defined(ASIO_HAS_MOVE)

  any_executor_base(any_executor_base&& other) ASIO_NOEXCEPT
    : object_fns_(other.object_fns_),
      target_fns_(other.target_fns_),
      blocking_(other.blocking_)
  {
    other.object_fns_ = object_fns_table<void>();
    other.target_fns_ = target_fns_table<void>();
    other.blocking_ = execution::blocking_t();
    object_fns_->move(*this, other);
    other.target_ = 0;
  }

  any_executor_base& operator=(
      any_executor_base&& other) ASIO_NOEXCEPT
  {
    if (this != &other)
    {
      object_fns_->destroy(*this);
      object_fns_ = other.object_fns_;
      other.object_fns_ = object_fns_table<void>();
      target_fns_ = other.target_fns_;
      other.target_fns_ = target_fns_table<void>();
      blocking_ = other.blocking_;
      other.blocking_ = execution::blocking_t();
      object_fns_->move(*this, other);
      other.target_ = 0;
    }
    return *this;
  }

#endif // defined(ASIO_HAS_MOVE)

  template <typename F>
  void execute(ASIO_MOVE_ARG(F) f) const
  {
    if (blocking_ == execution::blocking.always)
    {
      asio::detail::non_const_lvalue<F> f2(f);
      target_fns_->blocking_execute(*this, function_view(f2.value));
    }
    else
    {
      target_fns_->execute(*this,
          function(ASIO_MOVE_CAST(F)(f), std::allocator<void>()));
    }
  }

  template <typename Executor>
  Executor* target()
  {
    return static_cast<Executor*>(target_);
  }

  template <typename Executor>
  const Executor* target() const
  {
    return static_cast<Executor*>(target_);
  }

  const std::type_info& target_type() const
  {
    return target_fns_->target_type();
  }

protected:
  bool equal(const any_executor_base& other) const ASIO_NOEXCEPT
  {
    if (target_ == other.target_)
      return true;
    if (target_ && !other.target_)
      return false;
    if (!target_ && other.target_)
      return false;
    if (target_fns_ != other.target_fns_)
      return false;
    return target_fns_->equal(*this, other);
  }

  template <typename Ex>
  Ex& object()
  {
    return *static_cast<Ex*>(static_cast<void*>(&object_));
  }

  template <typename Ex>
  const Ex& object() const
  {
    return *static_cast<const Ex*>(static_cast<const void*>(&object_));
  }

  struct object_fns
  {
    void (*destroy)(any_executor_base&);
    void (*copy)(any_executor_base&, const any_executor_base&);
    void (*move)(any_executor_base&, any_executor_base&);
    const void* (*target)(const any_executor_base&);
  };

  static void destroy_void(any_executor_base&)
  {
  }

  static void copy_void(any_executor_base&, const any_executor_base&)
  {
  }

  static void move_void(any_executor_base&, any_executor_base&)
  {
  }

  static const void* target_void(const any_executor_base&)
  {
    return 0;
  }

  template <typename Obj>
  static const object_fns* object_fns_table(
      typename enable_if<
        is_same<Obj, void>::value
      >::type* = 0)
  {
    static const object_fns fns =
    {
      &any_executor_base::destroy_void,
      &any_executor_base::copy_void,
      &any_executor_base::move_void,
      &any_executor_base::target_void
    };
    return &fns;
  }

  static void destroy_shared(any_executor_base& ex)
  {
    typedef asio::detail::shared_ptr<void> type;
    ex.object<type>().~type();
  }

  static void copy_shared(any_executor_base& ex1, const any_executor_base& ex2)
  {
    typedef asio::detail::shared_ptr<void> type;
    new (&ex1.object_) type(ex2.object<type>());
    ex1.target_ = ex2.target_;
  }

  static void move_shared(any_executor_base& ex1, any_executor_base& ex2)
  {
    typedef asio::detail::shared_ptr<void> type;
    new (&ex1.object_) type(ASIO_MOVE_CAST(type)(ex2.object<type>()));
    ex1.target_ = ex2.target_;
    ex2.object<type>().~type();
  }

  static const void* target_shared(const any_executor_base& ex)
  {
    typedef asio::detail::shared_ptr<void> type;
    return ex.object<type>().get();
  }

  template <typename Obj>
  static const object_fns* object_fns_table(
      typename enable_if<
        is_same<Obj, asio::detail::shared_ptr<void> >::value
      >::type* = 0)
  {
    static const object_fns fns =
    {
      &any_executor_base::destroy_shared,
      &any_executor_base::copy_shared,
      &any_executor_base::move_shared,
      &any_executor_base::target_shared
    };
    return &fns;
  }

  template <typename Obj>
  static void destroy_object(any_executor_base& ex)
  {
    ex.object<Obj>().~Obj();
  }

  template <typename Obj>
  static void copy_object(any_executor_base& ex1, const any_executor_base& ex2)
  {
    new (&ex1.object_) Obj(ex2.object<Obj>());
    ex1.target_ = &ex1.object<Obj>();
  }

  template <typename Obj>
  static void move_object(any_executor_base& ex1, any_executor_base& ex2)
  {
    new (&ex1.object_) Obj(ASIO_MOVE_CAST(Obj)(ex2.object<Obj>()));
    ex1.target_ = &ex1.object<Obj>();
    ex2.object<Obj>().~Obj();
  }

  template <typename Obj>
  static const void* target_object(const any_executor_base& ex)
  {
    return &ex.object<Obj>();
  }

  template <typename Obj>
  static const object_fns* object_fns_table(
      typename enable_if<
        !is_same<Obj, void>::value
          && !is_same<Obj, asio::detail::shared_ptr<void> >::value
      >::type* = 0)
  {
    static const object_fns fns =
    {
      &any_executor_base::destroy_object<Obj>,
      &any_executor_base::copy_object<Obj>,
      &any_executor_base::move_object<Obj>,
      &any_executor_base::target_object<Obj>
    };
    return &fns;
  }

  typedef asio::detail::executor_function function;
  typedef asio::detail::executor_function_view function_view;

  struct target_fns
  {
    const std::type_info& (*target_type)();
    bool (*equal)(const any_executor_base&, const any_executor_base&);
    void (*execute)(const any_executor_base&, ASIO_MOVE_ARG(function));
    void (*blocking_execute)(const any_executor_base&, function_view);
  };

  static const std::type_info& target_type_void()
  {
    return typeid(void);
  }

  static bool equal_void(const any_executor_base&, const any_executor_base&)
  {
    return true;
  }

  static void execute_void(const any_executor_base&,
      ASIO_MOVE_ARG(function))
  {
    bad_executor ex;
    asio::detail::throw_exception(ex);
  }

  static void blocking_execute_void(const any_executor_base&, function_view)
  {
    bad_executor ex;
    asio::detail::throw_exception(ex);
  }

  template <typename Ex>
  static const target_fns* target_fns_table(
      typename enable_if<
        is_same<Ex, void>::value
      >::type* = 0)
  {
    static const target_fns fns =
    {
      &any_executor_base::target_type_void,
      &any_executor_base::equal_void,
      &any_executor_base::execute_void,
      &any_executor_base::blocking_execute_void
    };
    return &fns;
  }

  template <typename Ex>
  static const std::type_info& target_type_ex()
  {
    return typeid(Ex);
  }

  template <typename Ex>
  static bool equal_ex(const any_executor_base& ex1,
      const any_executor_base& ex2)
  {
    return *ex1.target<Ex>() == *ex2.target<Ex>();
  }

  template <typename Ex>
  static void execute_ex(const any_executor_base& ex,
      ASIO_MOVE_ARG(function) f)
  {
    ex.target<Ex>()->execute(ASIO_MOVE_CAST(function)(f));
  }

  template <typename Ex>
  static void blocking_execute_ex(const any_executor_base& ex, function_view f)
  {
    ex.target<Ex>()->execute(f);
  }

  template <typename Ex>
  static const target_fns* target_fns_table(
      typename enable_if<
        !is_same<Ex, void>::value
      >::type* = 0)
  {
    static const target_fns fns =
    {
      &any_executor_base::target_type_ex<Ex>,
      &any_executor_base::equal_ex<Ex>,
      &any_executor_base::execute_ex<Ex>,
      &any_executor_base::blocking_execute_ex<Ex>
    };
    return &fns;
  }

  template <typename Ex, class Prop>
  static void* query_fn_impl(const void*, const void*,
      typename enable_if<
        is_same<Ex, void>::value
      >::type*)
  {
    bad_executor ex;
    asio::detail::throw_exception(ex);
    return 0;
  }

  template <typename Ex, class Prop>
  static void* query_fn_impl(const void* ex, const void* prop,
      typename enable_if<
        !is_same<Ex, void>::value
          && (Prop::is_requirable || Prop::is_preferable
            || asio::can_query<const Ex&, const Prop&>::value)
          && is_same<typename Prop::polymorphic_query_result_type, void>::value
      >::type*)
  {
    asio::query(*static_cast<const Ex*>(ex),
        *static_cast<const Prop*>(prop));
    return 0;
  }

  template <typename Ex, class Prop>
  static void* query_fn_impl(const void* ex, const void* prop,
      typename enable_if<
        !is_same<Ex, void>::value
          && (Prop::is_requirable || Prop::is_preferable
            || asio::can_query<const Ex&, const Prop&>::value)
          && !is_same<typename Prop::polymorphic_query_result_type, void>::value
      >::type*)
  {
    return new typename Prop::polymorphic_query_result_type(
        asio::query(*static_cast<const Ex*>(ex),
            *static_cast<const Prop*>(prop)));
  }

  template <typename Ex, class Prop>
  static void* query_fn_impl(const void*, const void*, ...)
  {
    return new typename Prop::polymorphic_query_result_type();
  }

  template <typename Ex, class Prop>
  static void* query_fn(const void* ex, const void* prop)
  {
    return query_fn_impl<Ex, Prop>(ex, prop, 0);
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly require_fn_impl(const void*, const void*,
      typename enable_if<
        is_same<Ex, void>::value
      >::type*)
  {
    bad_executor ex;
    asio::detail::throw_exception(ex);
    return Poly();
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly require_fn_impl(const void* ex, const void* prop,
      typename enable_if<
        !is_same<Ex, void>::value && Prop::is_requirable
      >::type*)
  {
    return asio::require(*static_cast<const Ex*>(ex),
        *static_cast<const Prop*>(prop));
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly require_fn_impl(const void*, const void*, ...)
  {
    return Poly();
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly require_fn(const void* ex, const void* prop)
  {
    return require_fn_impl<Poly, Ex, Prop>(ex, prop, 0);
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly prefer_fn_impl(const void*, const void*,
      typename enable_if<
        is_same<Ex, void>::value
      >::type*)
  {
    bad_executor ex;
    asio::detail::throw_exception(ex);
    return Poly();
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly prefer_fn_impl(const void* ex, const void* prop,
      typename enable_if<
        !is_same<Ex, void>::value && Prop::is_preferable
      >::type*)
  {
    return asio::prefer(*static_cast<const Ex*>(ex),
        *static_cast<const Prop*>(prop));
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly prefer_fn_impl(const void*, const void*, ...)
  {
    return Poly();
  }

  template <typename Poly, typename Ex, class Prop>
  static Poly prefer_fn(const void* ex, const void* prop)
  {
    return prefer_fn_impl<Poly, Ex, Prop>(ex, prop, 0);
  }

  template <typename Poly>
  struct prop_fns
  {
    void* (*query)(const void*, const void*);
    Poly (*require)(const void*, const void*);
    Poly (*prefer)(const void*, const void*);
  };

private:
  template <typename Executor>
  static execution::blocking_t query_blocking(const Executor& ex, true_type)
  {
    return asio::query(ex, execution::blocking);
  }

  template <typename Executor>
  static execution::blocking_t query_blocking(const Executor&, false_type)
  {
    return execution::blocking_t();
  }

  template <typename Executor>
  void construct_object(Executor& ex, true_type)
  {
    object_fns_ = object_fns_table<Executor>();
    target_ = new (&object_) Executor(ASIO_MOVE_CAST(Executor)(ex));
  }

  template <typename Executor>
  void construct_object(Executor& ex, false_type)
  {
    object_fns_ = object_fns_table<asio::detail::shared_ptr<void> >();
    asio::detail::shared_ptr<Executor> p =
      asio::detail::make_shared<Executor>(
          ASIO_MOVE_CAST(Executor)(ex));
    target_ = p.get();
    new (&object_) asio::detail::shared_ptr<void>(
        ASIO_MOVE_CAST(asio::detail::shared_ptr<Executor>)(p));
  }

/*private:*/public:
//  template <typename...> friend class any_executor;

  typedef aligned_storage<
      sizeof(asio::detail::shared_ptr<void>),
      alignment_of<asio::detail::shared_ptr<void> >::value
    >::type object_type;

  object_type object_;
  const object_fns* object_fns_;
  void* target_;
  const target_fns* target_fns_;
  execution::blocking_t blocking_;
};

} // namespace detail

template <>
class any_executor<> : detail::any_executor_base
{
public:
  any_executor() ASIO_NOEXCEPT
    : detail::any_executor_base()
  {
  }

  template <ASIO_EXECUTION_EXECUTOR Executor>
  any_executor(Executor ex)
    : detail::any_executor_base(
        ASIO_MOVE_CAST(Executor)(ex), false_type())
  {
  }

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename... OtherSupportableProperties>
  any_executor(any_executor<OtherSupportableProperties...> other)
    : detail::any_executor_base(
        static_cast<const detail::any_executor_base&>(other))
  {
  }

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename U0, typename U1, typename U2, typename U3, typename U4>
  any_executor(any_executor<U0, U1, U2, U3, U4> other)
    : detail::any_executor_base(
        static_cast<const detail::any_executor_base&>(other))
  {
  }

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  any_executor(const any_executor& other) ASIO_NOEXCEPT
    : detail::any_executor_base(
        static_cast<const detail::any_executor_base&>(other))
  {
  }

#if defined(ASIO_HAS_MOVE)

  any_executor(any_executor&& other) ASIO_NOEXCEPT
    : detail::any_executor_base(
        static_cast<any_executor_base&&>(
          static_cast<any_executor_base&>(other)))
  {
  }

#endif // defined(ASIO_HAS_MOVE)

  using detail::any_executor_base::operator=;
  using detail::any_executor_base::execute;
  using detail::any_executor_base::target;
  using detail::any_executor_base::target_type;

  friend bool operator==(const any_executor& a,
      const any_executor& b) ASIO_NOEXCEPT
  {
    return a.equal(b);
  }

  friend bool operator!=(const any_executor& a,
      const any_executor& b) ASIO_NOEXCEPT
  {
    return !a.equal(b);
  }
};

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename... SupportableProperties>
class any_executor :
  public detail::any_executor_base,
  public detail::any_executor_props_base<
    any_executor<SupportableProperties...>,
      0, void(SupportableProperties...)>
{
public:
  any_executor() ASIO_NOEXCEPT
    : detail::any_executor_base(),
      prop_fns_(prop_fns_table<void>())
  {
  }

  template <ASIO_EXECUTION_EXECUTOR Executor>
  any_executor(Executor ex,
      typename enable_if<
        detail::any_executor_props_base<
          any_executor<SupportableProperties...>,
            0, void(SupportableProperties...)>::template
              is_valid_target<Executor>::value
      >::type* = 0)
    : detail::any_executor_base(ASIO_MOVE_CAST(Executor)(ex), false_type()),
      prop_fns_(prop_fns_table<Executor>())
  {
  }

  template <typename... OtherSupportableProperties>
  any_executor(any_executor<OtherSupportableProperties...> other,
      typename enable_if<
        detail::is_subset<any_executor,
          any_executor<OtherSupportableProperties...> >::value
      >::type* = 0)
    : detail::any_executor_base(ASIO_MOVE_CAST(
          any_executor<OtherSupportableProperties...>)(other), true_type()),
      prop_fns_(prop_fns_table<any_executor<OtherSupportableProperties...> >())
  {
  }

  template <typename... OtherSupportableProperties>
  any_executor(any_executor<OtherSupportableProperties...> other,
      typename enable_if<
        !detail::is_subset<any_executor,
          any_executor<OtherSupportableProperties...> >::value
      >::type* = 0) ASIO_DELETED;

  any_executor(const any_executor& other) ASIO_NOEXCEPT
    : detail::any_executor_base(
        static_cast<const detail::any_executor_base&>(other)),
      prop_fns_(other.prop_fns_)
  {
  }

  any_executor& operator=(const any_executor& other) ASIO_NOEXCEPT
  {
    if (this != &other)
    {
      prop_fns_ = other.prop_fns_;
      detail::any_executor_base::operator=(
          static_cast<detail::any_executor_base&>(other));
    }
    return *this;
  }

#if defined(ASIO_HAS_MOVE)

  any_executor(any_executor&& other) ASIO_NOEXCEPT
    : detail::any_executor_base(
        static_cast<any_executor_base&&>(
          static_cast<any_executor_base&>(other))),
      prop_fns_(other.prop_fns_)
  {
    other.prop_fns_ = prop_fns_table<void>();
  }

  any_executor& operator=(any_executor&& other) ASIO_NOEXCEPT
  {
    if (this != &other)
    {
      prop_fns_ = other.prop_fns_;
      detail::any_executor_base::operator=(
          static_cast<detail::any_executor_base&&>(
            static_cast<detail::any_executor_base&>(other)));
    }
    return *this;
  }

#endif // defined(ASIO_HAS_MOVE)

  using detail::any_executor_base::execute;
  using detail::any_executor_base::target;
  using detail::any_executor_base::target_type;
  using detail::any_executor_props_base<any_executor,
    0, void(SupportableProperties...)>::query;
  using detail::any_executor_props_base<any_executor,
    0, void(SupportableProperties...)>::require;

  friend bool operator==(const any_executor& a,
      const any_executor& b) ASIO_NOEXCEPT
  {
    return a.equal(b);
  }

  friend bool operator!=(const any_executor& a,
      const any_executor& b) ASIO_NOEXCEPT
  {
    return !a.equal(b);
  }

//private:
  template <typename Ex>
  static const prop_fns<any_executor>* prop_fns_table()
  {
    static const prop_fns<any_executor> fns[] =
    {
      {
        &detail::any_executor_base::query_fn<
            Ex, SupportableProperties>,
        &detail::any_executor_base::require_fn<
            any_executor, Ex, SupportableProperties>,
        &detail::any_executor_base::prefer_fn<
            any_executor, Ex, SupportableProperties>
      }...
    };
    return fns;
  }

  const prop_fns<any_executor>* prop_fns_;
};

template <typename... SupportableProperties>
struct is_executor<any_executor<SupportableProperties...> > : true_type
{
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#define ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS(n) \
  ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_##n

#define ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_1 \
  {  \
    &detail::any_executor_base::query_fn<Ex, T1>, \
    &detail::any_executor_base::require_fn<any_executor, Ex, T1>, \
    &detail::any_executor_base::prefer_fn<any_executor, Ex, T1> \
  }
#define ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_2 \
  ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_1, \
  { \
    &detail::any_executor_base::query_fn<Ex, T2>, \
    &detail::any_executor_base::require_fn<any_executor, Ex, T2>, \
    &detail::any_executor_base::prefer_fn<any_executor, Ex, T2> \
  }
#define ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_3 \
  ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_2, \
  { \
    &detail::any_executor_base::query_fn<Ex, T3>, \
    &detail::any_executor_base::require_fn<any_executor, Ex, T3>, \
    &detail::any_executor_base::prefer_fn<any_executor, Ex, T3> \
  }
#define ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_4 \
  ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_3, \
  { \
    &detail::any_executor_base::query_fn<Ex, T4>, \
    &detail::any_executor_base::require_fn<any_executor, Ex, T4>, \
    &detail::any_executor_base::prefer_fn<any_executor, Ex, T4> \
  }
#define ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_5 \
  ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_4, \
  { \
    &detail::any_executor_base::query_fn<Ex, T5>, \
    &detail::any_executor_base::require_fn<any_executor, Ex, T5>, \
    &detail::any_executor_base::prefer_fn<any_executor, Ex, T5> \
  }

#if defined(ASIO_HAS_MOVE)

# define ASIO_PRIVATE_ANY_EXECUTOR_MOVE_OPS \
  any_executor(any_executor&& other) ASIO_NOEXCEPT \
    : detail::any_executor_base( \
        static_cast<any_executor_base&&>( \
          static_cast<any_executor_base&>(other))), \
      prop_fns_(other.prop_fns_) \
  { \
    other.prop_fns_ = prop_fns_table<void>(); \
  } \
  \
  any_executor& operator=(any_executor&& other) ASIO_NOEXCEPT \
  { \
    if (this != &other) \
    { \
      prop_fns_ = other.prop_fns_; \
      detail::any_executor_base::operator=( \
          static_cast<detail::any_executor_base&&>( \
            static_cast<detail::any_executor_base&>(other))); \
    } \
    return *this; \
  } \
  /**/
#else // defined(ASIO_HAS_MOVE)

# define ASIO_PRIVATE_ANY_EXECUTOR_MOVE_OPS

#endif // defined(ASIO_HAS_MOVE)

#define ASIO_PRIVATE_ANY_EXECUTOR_DEF(n) \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  class any_executor<ASIO_VARIADIC_TARGS(n)> : \
    public detail::any_executor_base, \
    public detail::any_executor_props_base< \
      any_executor<ASIO_VARIADIC_TARGS(n)>, \
      0, void(ASIO_VARIADIC_TARGS(n))> \
  { \
  public: \
    any_executor() ASIO_NOEXCEPT \
      : detail::any_executor_base(), \
        prop_fns_(prop_fns_table<void>()) \
    { \
    } \
    \
    template <ASIO_EXECUTION_EXECUTOR Executor> \
    any_executor(Executor ex, \
      typename enable_if< \
        detail::any_executor_props_base< \
          any_executor<ASIO_VARIADIC_TARGS(n)>, \
            0, void(ASIO_VARIADIC_TARGS(n))>::template \
              is_valid_target<Executor>::value \
          && !detail::is_subset<any_executor<>, Executor>::value \
      >::type* = 0) \
      : detail::any_executor_base(ASIO_MOVE_CAST( \
            Executor)(ex), false_type()), \
        prop_fns_(prop_fns_table<Executor>()) \
    { \
    } \
    \
    any_executor(const any_executor& other) ASIO_NOEXCEPT \
      : detail::any_executor_base( \
          static_cast<const detail::any_executor_base&>(other)), \
        prop_fns_(other.prop_fns_) \
    { \
    } \
    \
    template <typename OtherAnyExecutor> \
    any_executor(OtherAnyExecutor other, \
        typename enable_if< \
          detail::is_subset<any_executor, OtherAnyExecutor>::value \
        >::type* = 0) \
      : detail::any_executor_base(ASIO_MOVE_CAST( \
            OtherAnyExecutor)(other), true_type()), \
        prop_fns_(prop_fns_table<OtherAnyExecutor>()) \
    { \
    } \
    \
    template <typename OtherAnyExecutor> \
    any_executor(OtherAnyExecutor other, \
        typename enable_if< \
          detail::is_subset<any_executor<>, OtherAnyExecutor>::value \
            && !detail::is_subset<any_executor, OtherAnyExecutor>::value \
        >::type* = 0) ASIO_DELETED; \
    \
    any_executor& operator=(const any_executor& other) ASIO_NOEXCEPT \
    { \
      if (this != &other) \
      { \
        prop_fns_ = other.prop_fns_; \
        detail::any_executor_base::operator=( \
            static_cast<detail::any_executor_base&>(other)); \
      } \
      return *this; \
    } \
    \
    ASIO_PRIVATE_ANY_EXECUTOR_MOVE_OPS \
    \
    using detail::any_executor_base::execute; \
    using detail::any_executor_base::target; \
    using detail::any_executor_base::target_type; \
    using detail::any_executor_props_base<any_executor, \
      0, void(ASIO_VARIADIC_TARGS(n))>::query; \
    using detail::any_executor_props_base<any_executor, \
      0, void(ASIO_VARIADIC_TARGS(n))>::require; \
    \
    friend bool operator==(const any_executor& a, \
        const any_executor& b) ASIO_NOEXCEPT \
    { \
      return a.equal(b); \
    } \
    \
    friend bool operator!=(const any_executor& a, \
        const any_executor& b) ASIO_NOEXCEPT \
    { \
      return !a.equal(b); \
    } \
    \
    template <typename Ex> \
    static const prop_fns<any_executor>* prop_fns_table() \
    { \
      static const prop_fns<any_executor> fns[] = \
      { \
        ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS(n) \
      }; \
      return fns; \
    } \
    \
    const prop_fns<any_executor>* prop_fns_; \
  }; \
  \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  struct is_executor<any_executor<ASIO_VARIADIC_TARGS(n)> > : true_type \
  { \
  }; \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_ANY_EXECUTOR_DEF)
#undef ASIO_PRIVATE_ANY_EXECUTOR_DEF
#undef ASIO_PRIVATE_ANY_EXECUTOR_MOVE_OPS
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_1
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_2
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_3
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_4
#undef ASIO_PRIVATE_ANY_EXECUTOR_PROP_FNS_5

#endif // if defined(ASIO_HAS_VARIADIC_TEMPLATES)

} // namespace execution
namespace traits {

#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)
#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template<typename Prop, typename... SupportableProperties>
struct query_member<
    execution::any_executor<SupportableProperties...>, Prop>
{
  static const bool is_valid = is_convertible<
    execution::any_executor<SupportableProperties...>*,
    execution::detail::any_executor_prop_query_tag<Prop>*>::value;
  static const bool is_noexcept = false;
  typedef typename Prop::polymorphic_query_result_type result_type;
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#define ASIO_PRIVATE_ANY_EXECUTOR_QUERY_MEMBER_DEF(n) \
  template<typename Prop, ASIO_VARIADIC_TPARAMS(n)> \
  struct query_member< \
      execution::any_executor<ASIO_VARIADIC_TARGS(n)>, Prop> \
  { \
    static const bool is_valid = is_convertible< \
      execution::any_executor<ASIO_VARIADIC_TARGS(n)>*, \
      execution::detail::any_executor_prop_query_tag<Prop>*>::value; \
    static const bool is_noexcept = false; \
    typedef typename Prop::polymorphic_query_result_type result_type; \
  }; \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_ANY_EXECUTOR_QUERY_MEMBER_DEF)
#undef ASIO_PRIVATE_ANY_EXECUTOR_QUERY_MEMBER_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)
#endif // !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)
#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template<typename Prop, typename... SupportableProperties>
struct require_member<
    execution::any_executor<SupportableProperties...>, Prop>
{
  static const bool is_valid = is_convertible<
    execution::any_executor<SupportableProperties...>*,
    execution::detail::any_executor_prop_require_tag<Prop>*>::value;
  static const bool is_noexcept = false;
  typedef execution::any_executor<SupportableProperties...> result_type;
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#define ASIO_PRIVATE_ANY_EXECUTOR_REQUIRE_MEMBER_DEF(n) \
  template<typename Prop, ASIO_VARIADIC_TPARAMS(n)> \
  struct require_member< \
      execution::any_executor<ASIO_VARIADIC_TARGS(n)>, Prop> \
  { \
    static const bool is_valid = is_convertible< \
      execution::any_executor<ASIO_VARIADIC_TARGS(n)>*, \
      execution::detail::any_executor_prop_require_tag<Prop>*>::value; \
    static const bool is_noexcept = false; \
    typedef execution::any_executor<ASIO_VARIADIC_TARGS(n)> result_type; \
  }; \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_ANY_EXECUTOR_REQUIRE_MEMBER_DEF)
#undef ASIO_PRIVATE_ANY_EXECUTOR_REQUIRE_MEMBER_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)
#endif // !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_PREFER_FREE_TRAIT)
#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template<typename Prop, typename... SupportableProperties>
struct prefer_free<
    execution::any_executor<SupportableProperties...>, Prop>
{
  static const bool is_valid = is_convertible<
    execution::any_executor<SupportableProperties...>*,
    execution::detail::any_executor_prop_require_tag<Prop>*>::value;
  static const bool is_noexcept = false;
  typedef execution::any_executor<SupportableProperties...> result_type;
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#define ASIO_PRIVATE_ANY_EXECUTOR_PREFER_FREE_DEF(n) \
  template<typename Prop, ASIO_VARIADIC_TPARAMS(n)> \
  struct prefer_free< \
      execution::any_executor<ASIO_VARIADIC_TARGS(n)>, Prop> \
  { \
    static const bool is_valid = is_convertible< \
      execution::any_executor<ASIO_VARIADIC_TARGS(n)>*, \
      execution::detail::any_executor_prop_require_tag<Prop>*>::value; \
    static const bool is_noexcept = false; \
    typedef execution::any_executor<ASIO_VARIADIC_TARGS(n)> result_type; \
  }; \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_ANY_EXECUTOR_PREFER_FREE_DEF)
#undef ASIO_PRIVATE_ANY_EXECUTOR_PREFER_FREE_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)
#endif // !defined(ASIO_HAS_DEDUCED_PREFER_FREE_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_ANY_EXECUTOR_HPP
