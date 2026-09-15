// Copyright 2026 Artiom Khachaturian
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
#pragma once // Concurrent.h
#include "MutexTraits.h"

#include <memory>
#include <utility>

namespace Bricks
{

template <class, class> class Concurrent;

// Operator-> drill-down effect (Execute-Around Pointer)
// Kevlin Henney: Function-Object and Execute-Around Pointer (https://hillside.net/europlop/HillsideEurope/Papers/ExecutingAroundSequences.pdf)
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Execute-Around_Pointer
namespace internal
{
#ifdef __cpp_concepts
// any smart pointer with suitable interface
template <typename T> concept SmartPtr = requires(T& ptr) {
    typename T::element_type; // always has type alias
    { ptr.get() } -> std::same_as<typename T::element_type*>; // has 'get' method
    { *ptr } -> std::same_as<typename T::element_type&>; // can be dereferenced
};

template <typename T> concept ReadOnly = std::is_const_v<T> ||
    (std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>) ||
    (std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>) ||
    (SmartPtr<T> && std::is_const_v<typename T::element_type>);
#endif

template <typename T>
struct GetElementType {
    using Type = std::remove_pointer_t<T>;
};

template <typename T>
struct GetElementType<T&> {
    using Type = std::remove_reference_t<T>;
};

#ifdef __cpp_concepts
template <SmartPtr T>
struct GetElementType<T> {
    using Type = typename T::element_type;
};
#else
template <typename T, typename Deleter>
struct GetElementType<std::unique_ptr<T, Deleter>> {
    using Type = typename T::element_type;
};

template <typename T>
struct GetElementType<std::shared_ptr<T>> {
    using Type = typename T::element_type;
};

template <typename T>
struct IsStdSmartPointer : std::false_type {};

template <typename T, typename Deleter>
struct IsStdSmartPointer<std::unique_ptr<T, Deleter>> : std::true_type {};

template <typename T>
struct IsStdSmartPointer<std::shared_ptr<T>> : std::true_type {};
#endif

// non-locking mutex
class StubMutex {
    template <class, class> friend class Bricks::Concurrent;
private:
    StubMutex() noexcept = default;
public:
    void lock() noexcept {}
    void unlock() noexcept {}
    bool try_lock() noexcept { return true; }
    void lock_shared() noexcept {}
    void unlock_shared() noexcept {}
    bool try_lock_shared() noexcept { return true; }
};

template <typename T, typename DefaultMutex>
struct DefaultMutexSelector {
#ifdef __cpp_concepts
    using Type = std::conditional_t<ReadOnly<T>, StubMutex, DefaultMutex>;
#else
    using Type = std::conditional_t<std::is_const_v<T>, StubMutex, DefaultMutex>;
#endif
};

} // internal

template <class, class> class ReadLocker;
template <class, class> class WriteLocker;

/**
 * @brief Thread-safe wrapper providing synchronized access to an underlying resource.
 * @tparam T The type of the resource being wrapped and protected.
 * @tparam TMutex The mutex type used for synchronization (defaults to std::shared_mutex).
 * @code
 * struct BankAccount {
 *     std::string owner;
 *     double balance;
 * };
 *
 * void print_balance(const Concurrent<BankAccount>& account) {
 *     // Read access (shared lock via operator->)
 *     std::cout << account->balance << std::endl;
 * }
 *
 * // Create a thread-safe wrapper
 * Concurrent<BankAccount> account{"Alice", 100.0};
 *
 * // Write access (exclusive lock via operator->)
 * account->balance += 50.0;
 * print_balance(account);
 *
 * // Scoped write access (exclusive lock via read())
 * {
 *     auto w = account.write();
 *     w->owner = "Bob";
 *     w->balance = 200.;
 * }
 *
 * // Scoped read access (shared lock via read())
 * {
 *     auto r = account.read();
 *     std::cout << r->owner << ": " << r->balance << std::endl;
 * }
 * @endcode
 */
template <class T, class TMutex = typename internal::DefaultMutexSelector<T, std::shared_mutex>::Type>
class Concurrent {
#ifdef __cpp_concepts
    static_assert(SharedMutex<TMutex>, "invalid mutex type");
#endif
public:
    using UnderlyingType = T;
    using MutexType = TMutex;
public:
    /**
     * @brief Constructs the internal resource by forwarding the provided arguments.
     * @param args Arguments to perfectly forward to T's constructor.
     */
    template<typename... Args>
    Concurrent(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : _t{std::forward<Args>(args)...} {}
    
    /**
     * @brief Constant arrow operator for read-only access.
     * @return A RAII read-lock guard (`ReadLocker`) over the resource.
     */
    const auto operator->() const { return read(); }

    /**
     * @brief Acquires a shared (read) lock on the resource.
     * @details Safe to call concurrently from multiple threads.
     * @return A RAII read-lock guard (`ReadLocker`) over the resource.
     */
    const auto read() const { return ReadLocker<T, TMutex>(_t, _m); }
    /**
     * @brief Mutable arrow operator for exclusive (write) access..
     * @return A RAII write-lock guard (`WriteLocker`) over the resource.
     */
#ifdef __cpp_concepts
    auto operator->() requires (!internal::ReadOnly<T>) { return write(); }
    auto write() requires (!internal::ReadOnly<T>) { return WriteLocker<T, TMutex>(_t, _m); }
#else
    auto operator->() { return write(); }
    auto write() { return WriteLocker<T, TMutex>(_t, _m); }
#endif
    
    // suppress copy/move
    Concurrent(const Concurrent&) = delete;
    Concurrent(Concurrent&&) noexcept = delete;
    Concurrent& operator=(const Concurrent&) = delete;
    Concurrent& operator=(Concurrent&&) noexcept = delete;
    
private:
    mutable TMutex _m;
    T _t;
};

/**
 * @brief Manages temporary access to a resource or smart pointer within a lock scope.
 * @details Automatically detects whether the stored type `T` is a smart pointer, raw pointer,
 *          or a standard object, and provides the appropriate raw pointer via `read()` or `write()`.
 */
template <class T> class LockerStorage {
    template <class, class> friend class ReadLocker;
    template <class, class> friend class WriteLocker;
public:
    /// Unwrapped inner element type (e.g., `U` for `std::unique_ptr<U>` or `T` for standard objects).
    using Type = typename internal::GetElementType<T>::Type;
public:
#ifdef __cpp_concepts
    /**
     * @brief Provides a read-only pointer to the underlying resource (C++20 version).
     * @return A pointer to the underlying element (`Type*` or `const Type*`).
     */
    const Type* read() const noexcept {
        if constexpr (internal::SmartPtr<T>) {
            return _t.get();
        } else if constexpr (std::is_pointer_v<T>) {
            return _t;
        } else {
            return std::addressof(_t);
        }
    }
    /**
     * @brief Provides a mutable pointer to the underlying resource (C++20 version).
     * @note Constrained by concepts; disabled if the underlying type `T` is read-only.
     * @return A mutable pointer to the underlying element (`Type*`).
     */
    Type* write() noexcept requires (!internal::ReadOnly<T>) {
        if constexpr (internal::SmartPtr<T>) {
            return _t.get();
        } else if constexpr (std::is_pointer_v<T>) {
            return _t;
        } else {
            return std::addressof(_t);
        }
    }
#else
    /**
     * @brief Provides a read-only pointer to the underlying resource (C++17 version).
     * @return A pointer to the underlying element (`Type*` or `const Type*`).
     */
    const Type* read() const noexcept {
        if constexpr (_is_std_smart_ptr) {
            return _t.get();
        } else if constexpr (std::is_pointer_v<T>) {
            return _t;
        } else {
            return std::addressof(_t);
        }
    }
    /**
     * @brief Provides a mutable pointer to the underlying resource (C++17 version).
     * @return A mutable pointer to the underlying element (`Type*`).
     */
    Type* write() noexcept {
        if constexpr (_is_std_smart_ptr) {
            return _t.get();
        } else if constexpr (std::is_pointer_v<T>) {
            return _t;
        } else {
            return std::addressof(_t);
        }
    }
#endif
private:
    /**
     * @brief Constructs a storage wrapper holding a reference to the resource.
     * @note Private constructor; accessible only by `ReadLocker` and `WriteLocker`.
     * @param t Reference to the underlying resource or smart pointer.
     */
    LockerStorage(T& t) noexcept : _t{t} {}
private:
#ifndef __cpp_concepts
    static inline constexpr bool _is_std_smart_ptr = internal::IsStdSmartPointer<T>::value;
#endif
    T& _t;
};

/**
 * @brief RAII guard providing thread-safe, shared (read-only) access to a resource.
 * @details Acquires a shared lock on the mutex upon construction and releases it upon destruction.
 *          Grants read-only access to the underlying resource via `LockerStorage`.
 */
template <class T, class TMutex>
class ReadLocker {
#ifdef __cpp_concepts
    static_assert(LockShared<TMutex>, "invalid shared mutex type");
#endif
    template <class, class> friend class Concurrent;
public:
    /// Unwrapped inner element type.
    using Type = typename LockerStorage<const T>::Type;
public:
    /// Move constructor (defaulted).
    ReadLocker(ReadLocker&&) noexcept = default;
    ~ReadLocker() = default;

    // Copy and move assignment are suppressed
    ReadLocker(const ReadLocker&) = delete;
    ReadLocker& operator=(const ReadLocker&) = delete;
    ReadLocker& operator=(ReadLocker&&) noexcept = delete;
    
    /**
     * @brief Accesses the underlying resource in a read-only manner.
     * @return A pointer to the underlying element (`Type*` or `const Type*`).
     */
    const Type* operator->() const noexcept { return _s.read(); }
private:
    /**
     * @brief Constructs the read locker by locking the mutex and wrapping the resource.
     * @note Private constructor; accessible only by `Concurrent`.
     * @param t Reference to the resource.
     * @param m Reference to the mutex.
     */
    ReadLocker(const T& t, TMutex& m) : _s{t}, _lock{m} {}
private:
    LockerStorage<const T> _s;
    std::shared_lock<TMutex> _lock;
};

/**
 * @brief RAII guard providing thread-safe, exclusive (write) access to a resource.
 * @details Acquires an exclusive lock on the mutex upon construction and releases it upon destruction.
 *          Grants mutable access to the underlying resource via `LockerStorage`.
 */
template <class T, class TMutex>
class WriteLocker {
#ifdef __cpp_concepts
    static_assert(LockExclusive<TMutex>, "invalid exclusive mutex type");
#endif
    template <class, class> friend class Concurrent;
public:
    /// Unwrapped inner element type.
    using Type = typename LockerStorage<T>::Type;
public:
    /// Move constructor (defaulted).
    WriteLocker(WriteLocker&&) noexcept = default;
    ~WriteLocker() = default;
    
    /**
     * @brief Accesses the underlying resource in a mutable manner.
     * @return A mutable pointer to the underlying element (`Type*`).
     */
    Type* operator->() noexcept { return _s.write(); }
    
    // Copy and move assignment are suppressed
    WriteLocker(const WriteLocker&) = delete;
    WriteLocker& operator=(const WriteLocker&) = delete;
    WriteLocker& operator=(WriteLocker&&) noexcept = delete;
private:
    WriteLocker(T& t, TMutex& m) : _s{t}, _lock{m} {}
private:
    LockerStorage<T> _s;
    std::unique_lock<TMutex> _lock;
};

} // namespace Bricks
