namespace nostd {
    // type_t, size_t
    template<class T>struct tag{using type=T;};
    template<class Tag>using type_t=typename Tag::type;
    using size_t=decltype(sizeof(int));
}
#ifndef _COMPILE_VS_NATIVE_
using nostd::size_t;
#endif

#ifdef _COMPILE_VS_NATIVE_

// Handle memory allocator that can keep track of memory leaks

#include <stdlib.h>
#include <stdio.h> 
#include <iostream>
#include <map>

//#define TRACK_MEMORY

std::map<void*, std::size_t> mem;
bool count_allocation = true;
int total_memory_usage = 0;

void* operator new(size_t size) 
{
    void* p = malloc(size); 
#ifdef TRACK_MEMORY
    if (count_allocation) {
        //std::cout << "Allocating " << size << " bytes at " << p << std::endl; 
        count_allocation = false;
        mem[p] = size;
        total_memory_usage += size;
        count_allocation = true;
    }
#endif
    return p; 
} 
  
void operator delete(void* p) 
{
    //printf("Deallocating %p\n", p);
#ifdef TRACK_MEMORY
    if (count_allocation && mem.count(p)) {
        size_t bytes = mem[p];
        mem.erase(p);
        total_memory_usage -= bytes;
        //std::cout << "Deallocating " << bytes << " bytes at " << p << std::endl; 
        std::cout << total_memory_usage << " remaining" << std::endl;
    }
#endif
    free(p); 
}

#endif

#ifndef _COMPILE_VS_NATIVE_
// Externs
struct string;
extern "C" {
    [[noreturn]] extern void panic(string* s);
    extern void print(string* s);
}

// Default placement versions of operator new.
inline void* operator new(nostd::size_t, void* __p) throw() { return __p; }
inline void* operator new[](nostd::size_t, void* __p) throw() { return __p; }

// Default placement versions of operator delete.
inline void  operator delete  (void*, void*) throw() { }
inline void  operator delete[](void*, void*) throw() { }
#endif

namespace nostd {
    // https://stackoverflow.com/questions/32074410/stdfunction-bind-like-type-erasure-without-standard-c-library

    // Move
    template<class T>
    T&& move(T&t){return static_cast<T&&>(t);}

    // Swap
    template<typename T> void swap(T& t1, T& t2) {
        T temp = nostd::move(t1); // or T temp(std::move(t1));
        t1 = nostd::move(t2);
        t2 = nostd::move(temp);
    }

    // Forward
    template<class T>
    struct remove_reference:tag<T>{};
    template<class T>
    struct remove_reference<T&>:tag<T>{};
    template<class T>using remove_reference_t=type_t<remove_reference<T>>;

    template<class T>
    T&& forward( remove_reference_t<T>& t ) {
    return static_cast<T&&>(t);
    }
    template<class T>
    T&& forward( remove_reference_t<T>&& t ) {
    return static_cast<T&&>(t);
    }

    // Decay
    template<class T>
    struct remove_const:tag<T>{};
    template<class T>
    struct remove_const<T const>:tag<T>{};

    template<class T>
    struct remove_volatile:tag<T>{};
    template<class T>
    struct remove_volatile<T volatile>:tag<T>{};

    template<class T>
    struct remove_cv:remove_const<type_t<remove_volatile<T>>>{};


    template<class T>
    struct decay3:remove_cv<T>{};
    template<class R, class...Args>
    struct decay3<R(Args...)>:tag<R(*)(Args...)>{};
    template<class T>
    struct decay2:decay3<T>{};
    template<class T, size_t N>
    struct decay2<T[N]>:tag<T*>{};

    template<class T>
    struct decay:decay2<remove_reference_t<T>>{};

    template<class T>
    using decay_t=type_t<decay<T>>;

    // is_convertable
    template<class T>
    T declval(); // no implementation

    template<class T, T t>
    struct integral_constant{
    static constexpr T value=t;
    constexpr integral_constant() {};
    constexpr operator T()const{ return value; }
    constexpr T operator()()const{ return value; }
    };
    template<bool b>
    using bool_t=integral_constant<bool, b>;
    using true_type=bool_t<true>;
    using false_type=bool_t<false>;

    template<class...>struct voider:tag<void>{};
    template<class...Ts>using void_t=type_t<voider<Ts...>>;

    namespace details {
    template<template<class...>class Z, class, class...Ts>
    struct can_apply:false_type{};
    template<template<class...>class Z, class...Ts>
    struct can_apply<Z, void_t<Z<Ts...>>, Ts...>:true_type{};
    }
    template<template<class...>class Z, class...Ts>
    using can_apply = details::can_apply<Z, void, Ts...>;

    namespace details {
    template<class From, class To>
    using try_convert = decltype( To{declval<From>()} );
    }
    template<class From, class To>
    struct is_convertible : can_apply< details::try_convert, From, To > {};
    template<>
    struct is_convertible<void,void>:true_type{};

    // enable_if
    template<bool, class=void>
    struct enable_if {};
    template<class T>
    struct enable_if<true, T>:tag<T>{};
    template<bool b, class T=void>
    using enable_if_t=type_t<enable_if<b,T>>;

    // result_of
    namespace details {
    template<class F, class...Args>
    using invoke_t = decltype( declval<F>()(declval<Args>()...) );

    template<class Sig,class=void>
    struct result_of {};
    template<class F, class...Args>
    struct result_of<F(Args...), void_t< invoke_t<F, Args...> > >:
        tag< invoke_t<F, Args...> >
    {};
    }
    template<class Sig>
    using result_of = details::result_of<Sig>;
    template<class Sig>
    using result_of_t=type_t<result_of<Sig>>;

    // aligned_storage
    template<size_t size, size_t align>
    struct alignas(align) aligned_storage_t {
    char buff[size];
    };

    // is_same
    template<class A, class B>
    struct is_same:false_type{};
    template<class A>
    struct is_same<A,A>:true_type{};

    // functional
    template<class Sig, size_t sz, size_t algn>
    struct small_task;

    template<class R, class...Args, size_t sz, size_t algn>
    struct small_task<R(Args...), sz, algn>{
        struct vtable_t {
            void(*mover)(void* src, void* dest);
            void(*copyer)(const void* src, void* dest);
            void(*destroyer)(void*);
            R(*invoke)(void const* t, Args&&...args);
            template<class T>
            static vtable_t const* get() {
                static const vtable_t table = {
                    // Mover
                    [](void* src, void*dest) {
                        new(dest) T(nostd::move(*static_cast<T*>(src)));
                    },
                    // Copyer
                    [](const void* src, void*dest) {
                        new(dest) T(*static_cast<T const*>(src));
                    },
                    // Destroyer
                    [](void* t){ static_cast<T*>(t)->~T(); },
                    // Invoker
                    [](void const* t, Args&&...args)->R {
                        return (*static_cast<T const*>(t))(nostd::forward<Args>(args)...);
                    }
                };
                return &table;
            }
        };
        vtable_t const* table = nullptr;
        nostd::aligned_storage_t<sz, algn> data;
        template<class F,
            class dF=nostd::decay_t<F>,
            // don't use this ctor on own type:
            nostd::enable_if_t<!nostd::is_same<dF, small_task>{}>* = nullptr,
            // use this ctor only if the call is legal:
            nostd::enable_if_t<nostd::is_convertible<
            nostd::result_of_t<dF const&(Args...)>, R
            >{}>* = nullptr
        >
        small_task( F&& f ): table( vtable_t::template get<dF>() )
        {
            // a higher quality small_task would handle null function pointers
            // and other "nullable" callables, and construct as a null small_task

            static_assert( sizeof(dF) <= sz, "object too large" );
            static_assert( alignof(dF) <= algn, "object too aligned" );
            new(&data) dF(nostd::forward<F>(f));
        }
        // I find this overload to be useful, as it forces some
        // functions to resolve their overloads nicely:
        // small_task( R(*)(Args...) )
        ~small_task() {
            if (table) {
                table->destroyer(&data);
            }
        }
        small_task(const small_task& o):
            table(o.table)
        {
            if (table) {
                table->copyer(&o.data, &data);
            }
        }
        small_task(small_task&& o):
            table(o.table)
        {
            if (table) {
                table->mover(&o.data, &data);
            }
        }
        small_task(){}
        small_task& operator=(const small_task& o){
            // this is a bit rude and not very exception safe
            // you can do better:
            this->~small_task();
            new(this) small_task( o );
            return *this;
        }
        small_task& operator=(small_task&& o){
            // this is a bit rude and not very exception safe
            // you can do better:
            this->~small_task();
            new(this) small_task( nostd::move(o) );
            return *this;
        }
        explicit operator bool() const {
            return table;
        }
        R operator()(Args...args) const {
            return table->invoke(&data, nostd::forward<Args>(args)...);
        }
    };

    template<class Sig>
    using function = small_task<Sig, sizeof(void*)*4, alignof(void*) >;
}

#ifdef _COMPILE_VS_NATIVE_
#define standard std
#else
#define standard nostd
#endif
