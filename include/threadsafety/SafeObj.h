// Copyright 2025 Artiom Khachaturian
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once // SafeObj.h
#include "MutexTraits.h"

// Do magic! Creates a unique name using the line number
#define SAFE_OBJ_NAME(prefix) SAFE_OBJ_JOIN(prefix, __LINE__)

#define SAFE_OBJ_JOIN(symbol1, symbol2) DO_SAFE_OBJ_JOIN(symbol1, symbol2)

#define DO_SAFE_OBJ_JOIN(symbol1, symbol2) symbol1##symbol2

#define LOCK_READ_SAFE_OBJ(object) const typename decltype(object)::ReadLock \
    SAFE_OBJ_NAME(rl)(object.mutex()) \

#define LOCK_WRITE_SAFE_OBJ(object) const typename decltype(object)::WriteLock \
    SAFE_OBJ_NAME(wl)(object.mutex()) \

namespace Bricks
{

// Performance tests for different kind of mutexes:
//
// 1 million iterations, release type build, single thread
// std::mutex           - LOCK_READ_SAFE_OBJ 19-21 ms  LOCK_WRITE_SAFE_OBJ 19-21 ms
// std::recursive_mutex - LOCK_READ_SAFE_OBJ 32-33 ms, LOCK_WRITE_SAFE_OBJ 32-33 ms
// std::shared_mutex    - LOCK_READ_SAFE_OBJ 41 ms,    LOCK_WRITE_SAFE_OBJ 43 ms

/**
 * @brief A thread-safe wrapper for an object with customizable mutex and locking traits.
 *
 * The `SafeObj` class is designed to provide safe and unsafe access to an object of type `T`
 * in multithreaded environments. It uses customizable mutex types and traits for locking
 * and ensures proper synchronization where necessary.
 *
 * @tparam T The type of the encapsulated object.
 * @tparam TMutexType The type of the mutex used for synchronization (defaults to `std::recursive_mutex`).
 * @tparam TMutexTraits The traits class that defines locking behavior (defaults to `MutexTraits<TMutexType>`).
 */
template <typename T,
          class TMutexType = std::recursive_mutex,
          class TMutexTraits = MutexTraits<TMutexType>>
class SafeObj
{
public:
    /// @brief Alias for the encapsulated object type.
    using Type = T;

    /// @brief Alias for the mutex type used for synchronization.
    using Mutex = TMutexType;

    /// @brief Alias for the traits type used for defining lock types.
    using Traits = TMutexTraits;

    /// @brief Alias for the type of lock used for writing.
    using WriteLock = typename Traits::WriteLock;

    /// @brief Alias for the type of lock used for reading.
    using ReadLock = typename Traits::ReadLock;

public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `SafeObj` with default-initialized internal object.
     */
    SafeObj() = default;

    /**
     * @brief Constructs a `SafeObj` with an initial value.
     *
     * @param val The initial value for the encapsulated object.
     */
    explicit SafeObj(T val);

    /**
     * @brief Move constructor.
     *
     * Transfers ownership of the internal object from a temporary `SafeObj`.
     *
     * @param tmp The temporary `SafeObj` to move from.
     */
    explicit SafeObj(SafeObj&& tmp) noexcept;

    /**
     * @brief Constructs a `SafeObj` with variadic arguments.
     *
     * The arguments are forwarded to the constructor of the encapsulated object.
     *
     * @tparam Args Variadic template arguments for the encapsulated object's constructor.
     * @param args Arguments to forward.
     */
    template <class... Args>
    SafeObj(Args&&... args);

    /// @brief Copy constructor is disabled to prevent copying.
    SafeObj(const SafeObj&) = delete;

    /// @brief Copy assignment is disabled to prevent copying.
    SafeObj& operator=(const SafeObj&) = delete;

    /**
     * @brief Move assignment operator.
     *
     * Transfers ownership of the internal object from another `SafeObj`.
     *
     * @param tmp The temporary `SafeObj` to move from.
     * @return A reference to this `SafeObj`.
     */
    SafeObj& operator=(SafeObj&& tmp) noexcept;

    /**
     * @brief Assigns a new value to the encapsulated object.
     *
     * @tparam U The type of the new value (defaults to `T`).
     * @param src The new value to assign.
     * @return A reference to this `SafeObj`.
     */
    template <typename U = T>
    SafeObj& operator=(U src) noexcept;

    /**
     * @brief Gets a reference to the internal mutex.
     *
     * @return A reference to the mutex used for synchronization.
     */
    auto& mutex() const noexcept { return _mtx; }

    /**
     * @brief Implicitly converts the object to a constant reference.
     *
     * @return A constant reference to the encapsulated object.
     */
    operator const T&() const noexcept { return constRef(); }

    /**
     * @brief Implicitly converts the object to a mutable reference.
     *
     * @return A mutable reference to the encapsulated object.
     */
    operator T&() noexcept { return ref(); }

    /**
     * @brief Gets a constant reference to the encapsulated object.
     *
     * @return A constant reference to the encapsulated object.
     */
    const T& constRef() const noexcept { return _obj; }

    /**
     * @brief Gets a mutable reference to the encapsulated object.
     *
     * @return A mutable reference to the encapsulated object.
     */
    T& ref() noexcept { return _obj; }

    /**
     * @brief Takes ownership of the encapsulated object.
     *
     * This moves the internal object out of the `SafeObj` instance, leaving it empty.
     *
     * @return The encapsulated object.
     */
    T take() noexcept { return std::move(_obj); }

    /**
     * @brief Exchanges the encapsulated object with the provided object.
     *
     * @param obj The object to exchange with.
     * @return The old encapsulated object.
     */
    T exchange(T obj) {
        std::swap(_obj, obj);
        return obj; // old
    }

    /**
     * @brief Accesses the encapsulated object via pointer semantics.
     *
     * @return A pointer to the encapsulated object.
     */
    T* operator->() noexcept { return &_obj; }

    /**
     * @brief Accesses the encapsulated object via pointer semantics (constant).
     *
     * @return A constant pointer to the encapsulated object.
     */
    const T* operator->() const noexcept { return &_obj; }

    /**
     * @brief Safely assigns a new value to the encapsulated object with locking.
     *
     * @tparam U The type of the new value (defaults to `T`).
     * @param src The new value to assign.
     */
    template <typename U = T>
    void operator()(U src);

    /**
     * @brief Safely retrieves the encapsulated object with locking.
     *
     * @return The encapsulated object.
     */
    T operator()() const;

private:
    /// @brief The mutex used for synchronization.
    mutable TMutexType _mtx;

    /// @brief The encapsulated object.
    T _obj;
};

// impl. of SafeObj
template <typename T, class TMutexType, class TMutexTraits>
inline SafeObj<T, TMutexType, TMutexTraits>::SafeObj(T val)
    : _obj(std::move(val))
{
}

template <typename T, class TMutexType, class TMutexTraits>
inline SafeObj<T, TMutexType, TMutexTraits>::SafeObj(SafeObj&& tmp) noexcept
    : _obj(std::move(tmp._obj))
{
}

template <typename T, class TMutexType, class TMutexTraits>
template <class... Args>
inline SafeObj<T, TMutexType, TMutexTraits>::SafeObj(Args&&... args)
    : _obj(std::forward<Args>(args)...)
{
}

template <typename T, class TMutexType, class TMutexTraits>
inline SafeObj<T, TMutexType, TMutexTraits>& SafeObj<T, TMutexType, TMutexTraits>::
    operator = (SafeObj&& tmp) noexcept
{
    if (&tmp != this) {
        _obj = std::move(tmp._obj);
    }
    return *this;
}

template <typename T, class TMutexType, class TMutexTraits>
template <typename U>
inline SafeObj<T, TMutexType, TMutexTraits>& SafeObj<T, TMutexType, TMutexTraits>::
    operator = (U src) noexcept
{
    _obj = std::move(src);
    return *this;
}

template <typename T, class TMutexType, class TMutexTraits>
template <typename U>
void SafeObj<T, TMutexType, TMutexTraits>::operator () (U src)
{
    const WriteLock lock(mutex());
    _obj = std::move(src);
}

template <typename T, class TMutexType, class TMutexTraits>
T SafeObj<T, TMutexType, TMutexTraits>::operator() () const
{
    const ReadLock lock(mutex());
    return _obj;
}

} // namespace Bricks
