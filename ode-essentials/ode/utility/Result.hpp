
#include "Result.h"

#include "../utils.h"

namespace ode {

template <typename T, typename E>
Result<T, E>::Result() { }

template <typename T, typename E>
Result<T, E>::Result(const T &value) {
    new(data.value) T(value);
    content = VALUE;
}

template <typename T, typename E>
Result<T, E>::Result(T &&value) {
    new(data.value) T((T &&) value);
    content = VALUE;
}

template <typename T, typename E>
Result<T, E>::Result(const E &error) {
    new(data.error) E(error);
    content = ERROR;
}

template <typename T, typename E>
Result<T, E>::Result(E &&error) {
    new(data.error) E((E &&) error);
    content = ERROR;
}

template <typename T, typename E>
Result<T, E>::Result(const Result<T, E> &orig) {
    switch (orig.content) {
        case NONE:
            break;
        case VALUE:
            new(data.value) T(*reinterpret_cast<const T *>(orig.data.value));
            break;
        case ERROR:
            new(data.error) E(*reinterpret_cast<const E *>(orig.data.error));
            break;
    }
    content = orig.content;
}

template <typename T, typename E>
Result<T, E>::Result(Result<T, E> &&orig) {
    switch (orig.content) {
        case NONE:
            break;
        case VALUE:
            new(data.value) T((T &&) *reinterpret_cast<T *>(orig.data.value));
            break;
        case ERROR:
            new(data.error) E((E &&) *reinterpret_cast<E *>(orig.data.error));
            break;
    }
    content = orig.content;
}

template <typename T, typename E>
Result<T, E>::~Result() {
    switch (content) {
        case NONE:
            break;
        case VALUE:
            reinterpret_cast<T *>(data.value)->~T();
            break;
        case ERROR:
            reinterpret_cast<E *>(data.error)->~E();
            break;
    }
}

template <typename T, typename E>
Result<T, E> &Result<T, E>::operator=(const T &value) {
    switch (content) {
        case VALUE:
            *reinterpret_cast<T *>(data.value) = value;
            break;
        case ERROR:
            reinterpret_cast<E *>(data.error)->~E();
            content = NONE;
            // fallthrough
        case NONE:
            new(data.value) T(value);
            content = VALUE;
    }
    return *this;
}

template <typename T, typename E>
Result<T, E> &Result<T, E>::operator=(T &&value) {
    switch (content) {
        case VALUE:
            *reinterpret_cast<T *>(data.value) = (T &&) value;
            break;
        case ERROR:
            reinterpret_cast<E *>(data.error)->~E();
            content = NONE;
            // fallthrough
        case NONE:
            new(data.value) T((T &&) value);
            content = VALUE;
    }
    return *this;
}

template <typename T, typename E>
Result<T, E> &Result<T, E>::operator=(const E &error) {
    switch (content) {
        case ERROR:
            *reinterpret_cast<E *>(data.error) = error;
            break;
        case VALUE:
            reinterpret_cast<T *>(data.value)->~T();
            content = NONE;
            // fallthrough
        case NONE:
            new(data.error) E(error);
            content = ERROR;
    }
    return *this;
}

template <typename T, typename E>
Result<T, E> &Result<T, E>::operator=(E &&error) {
    switch (content) {
        case ERROR:
            *reinterpret_cast<E *>(data.error) = (E &&) error;
            break;
        case VALUE:
            reinterpret_cast<T *>(data.value)->~T();
            content = NONE;
            // fallthrough
        case NONE:
            new(data.error) E((E &&) error);
            content = ERROR;
    }
    return *this;
}

template <typename T, typename E>
Result<T, E> &Result<T, E>::operator=(const Result<T, E> &orig) {
    if (content == orig.content) {
        switch (orig.content) {
            case NONE:
                break;
            case VALUE:
                *reinterpret_cast<T *>(data.value) = *reinterpret_cast<const T *>(orig.data.value);
                break;
            case ERROR:
                *reinterpret_cast<E *>(data.error) = *reinterpret_cast<const E *>(orig.data.error);
                break;
        }
    } else {
        switch (content) {
            case NONE:
                break;
            case VALUE:
                reinterpret_cast<T *>(data.value)->~T();
                break;
            case ERROR:
                reinterpret_cast<E *>(data.error)->~E();
                break;
        }
        content = NONE;
        switch (orig.content) {
            case NONE:
                break;
            case VALUE:
                new(data.value) T(*reinterpret_cast<const T *>(orig.data.value));
                break;
            case ERROR:
                new(data.error) E(*reinterpret_cast<const E *>(orig.data.error));
                break;
        }
        content = orig.content;
    }
    return *this;
}

template <typename T, typename E>
Result<T, E> &Result<T, E>::operator=(Result<T, E> &&orig) {
    if (content == orig.content) {
        switch (orig.content) {
            case NONE:
                break;
            case VALUE:
                *reinterpret_cast<T *>(data.value) = (T &&) *reinterpret_cast<T *>(orig.data.value);
                break;
            case ERROR:
                *reinterpret_cast<E *>(data.error) = (E &&) *reinterpret_cast<E *>(orig.data.error);
                break;
        }
    } else {
        switch (content) {
            case NONE:
                break;
            case VALUE:
                reinterpret_cast<T *>(data.value)->~T();
                break;
            case ERROR:
                reinterpret_cast<E *>(data.error)->~E();
                break;
        }
        content = NONE;
        switch (orig.content) {
            case NONE:
                break;
            case VALUE:
                new(data.value) T((T &&) *reinterpret_cast<T *>(orig.data.value));
                break;
            case ERROR:
                new(data.error) E((E &&) *reinterpret_cast<E *>(orig.data.error));
                break;
        }
        content = orig.content;
    }
    return *this;
}

template <typename T, typename E>
bool Result<T, E>::empty() const {
    return content == NONE;
}

template <typename T, typename E>
bool Result<T, E>::success() const {
    return content == VALUE;
}

template <typename T, typename E>
bool Result<T, E>::failure() const {
    return content == ERROR;
}

template <typename T, typename E>
const T &Result<T, E>::value() const {
    ODE_ASSERT(content == VALUE);
    return *reinterpret_cast<const T *>(data.value);
}

template <typename T, typename E>
T &Result<T, E>::value() {
    ODE_ASSERT(content == VALUE);
    return *reinterpret_cast<T *>(data.value);
}

template <typename T, typename E>
const E &Result<T, E>::error() const {
    ODE_ASSERT(content == ERROR);
    return *reinterpret_cast<const E *>(data.error);
}

template <typename T, typename E>
E &Result<T, E>::error() {
    ODE_ASSERT(content == ERROR);
    return *reinterpret_cast<E *>(data.error);
}

template <typename T, typename E>
Result<T, E>::operator bool() const {
    return success();
}

}
