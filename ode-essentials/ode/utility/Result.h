
#pragma once

#include <type_traits>

namespace ode {

/// Result may either hold the actual result T or an error value E, to be used as function return value
template <typename T, typename E>
class Result {

public:
    Result();
    Result(const T &value);
    Result(T &&value);
    Result(const E &error);
    Result(E &&error);
    Result(const Result<T, E> &orig);
    Result(Result<T, E> &&orig);

    /// Enables implicit conversions for T without ambiguity
    template <typename U, std::enable_if_t<std::conjunction_v<
        std::negation<std::is_same<U, T> >,
        std::negation<std::is_same<U, E> >,
        std::is_constructible<T, U &&>,
        std::is_convertible<U &&, T>,
        std::negation<std::is_constructible<E, U &&> >
    >, bool> = true>
    inline Result(U &&value) : Result(T((U &&) value)) { }

    /// Enables implicit conversions for E without ambiguity
    template <typename U, std::enable_if_t<std::conjunction_v<
        std::negation<std::is_same<U, E> >,
        std::negation<std::is_same<U, T> >,
        std::is_constructible<E, U &&>,
        std::is_convertible<U &&, E>,
        std::negation<std::is_constructible<T, U &&> >
    >, bool> = true>
    inline Result(U &&value) : Result(E((U &&) value)) { }

    ~Result();
    Result<T, E> &operator=(const T &value);
    Result<T, E> &operator=(T &&value);
    Result<T, E> &operator=(const E &error);
    Result<T, E> &operator=(E &&error);
    Result<T, E> &operator=(const Result<T, E> &orig);
    Result<T, E> &operator=(Result<T, E> &&orig);
    /// Returns true if neither result value nor error value is set
    bool empty() const;
    /// Returns true if the result value is set
    bool success() const;
    /// Returns true if the error value is set
    bool failure() const;
    /// Returns result value - caller must make sure the value is set!
    const T &value() const;
    /// Returns mutable result value - caller must make sure the value is set!
    T &value();
    /// Returns error value - caller must make sure the error value is set!
    const E &error() const;
    /// Returns mutable error value - caller must make sure the error value is set!
    E &error();
    /// Returns true if the result value is set (equivalent to success())
    explicit operator bool() const;

private:
    enum {
        NONE,
        VALUE,
        ERROR
    } content = NONE;
    union {
        alignas(T) unsigned char value[sizeof(T)];
        alignas(E) unsigned char error[sizeof(E)];
    } data;

};

}

#include "Result.hpp"
