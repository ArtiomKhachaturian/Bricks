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
#pragma once // Listener.h
#include "Invoke.h"
#include "ListenersMutexSelector.h"
#include "SafeObjAliases.h"
#include <atomic>
#include <memory>

namespace Bricks
{

/**
 * @brief A threadsafe wrapper for managing listener objects with mutex-based locking.
 *
 * The `Listener` class provides an interface for managing listener objects in multithreaded
 * environments. It ensures safe access with mutex locks while allowing flexibility through
 * customizable operations, such as invoking methods on the listener or resetting it.
 *
 * @tparam T The type of the listener object being managed.
 * @tparam ThreadSafe If true, enables thread-safe operations using a mutex (default: true).
 *                    If false, no mutex locking is performed (via `StubMutex`).
 */
template <typename T, bool ThreadSafe = true>
class Listener
{
    /// @brief Defines the type of mutex based on the thread-safety configuration.
    using MutexType = typename ListenersMutexSelector<ThreadSafe>::MutexType;
    using ReadLock = typename MutexTraits<MutexType>::ReadLock;
    using WriteLock = typename MutexTraits<MutexType>::WriteLock;
    using Impl = SafeObj<T, MutexType>;
public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `Listener` instance.
     */
    Listener();
    
    /**
     * @brief Constructs a `Listener` with a null listener.
     *
     * @param null A placeholder for initializing with a null listener (`std::nullptr_t`).
     */
    Listener(std::nullptr_t);

    /**
     * @brief Constructs a `Listener` with the specified listener object.
     *
     * @param listener The listener object to manage.
     */
    explicit Listener(T listener);

    /// @brief Copy & move assignment are disabled.
    Listener(Listener&& tmp) = delete;
    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

    /**
     * @brief Destructor.
     *
     * Ensures the listener is properly reset upon destruction.
     */
    ~Listener() { reset(); }

    /**
     * @brief Sets the listener object.
     *
     * @tparam U The type of the listener object (defaults to `T`).
     * @param listener The listener object to set.
     */
    template <typename U = T>
    void set(U listener = {});

    /**
     * @brief Resets the listener object to null.
     */
    void reset() { set({}); }

    /**
     * @brief Checks if the listener object is empty.
     *
     * A listener is considered empty if it has not been set or has been reset to null.
     *
     * @return `true` if the listener is empty, otherwise `false`.
     */
    bool empty() const noexcept;

    /**
     * @brief Invokes a method on the listener object with the specified arguments.
     *
     * @tparam Method The type of the method to invoke.
     * @tparam Args The types of the arguments to pass to the method.
     * @param method The method pointer to invoke on the listener.
     * @param args The arguments to pass to the method.
     */
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;

    /**
     * @brief Invokes a method on the listener object with the specified arguments, returning a result.
     *
     * @tparam R The return type of the method being invoked.
     * @tparam Method The type of the method to invoke.
     * @tparam Args The types of the arguments to pass to the method.
     * @param method The method pointer to invoke on the listener.
     * @param args The arguments to pass to the method.
     * @return The result of the method invocation.
     */
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const;

    /**
     * @brief Assigns a new listener object.
     *
     * @tparam U The type of the listener object (defaults to `T`).
     * @param listener The new listener object to assign.
     * @return A reference to this `Listener`.
     */
    template <typename U = T>
    Listener& operator=(U listener) noexcept;

    /**
     * @brief Checks if the listener is valid (non-empty).
     *
     * This operator allows the `Listener` to be used in a boolean context.
     *
     * @return `true` if the listener is valid, otherwise `false`.
     */
    explicit operator bool() const noexcept { return !empty(); }
private:
    /// @brief Threadsafe wrapper for managing the listener object.
    const std::shared_ptr<Impl> _listener;
};

/**
 * @brief Specialization of the `Listener` class for `std::shared_ptr` objects.
 *
 * This specialization provides lock-free management of listener objects stored
 * as `std::shared_ptr`. It supports operations similar to the base `Listener` class.
 *
 * @tparam T The type of the object managed by the shared pointer.
 * @tparam ThreadSafe If true, enables thread-safe operations using a atomic operations (default: true).
 *                    If false, no atomic operations are performed.
 */
template <typename T, bool ThreadSafe>
class Listener<std::shared_ptr<T>, ThreadSafe>
{
public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `Listener` instance.
     */
    Listener() = default;

    /**
     * @brief Constructs a `Listener` with a null listener.
     *
     * @param null A placeholder for initializing with a null listener (`std::nullptr_t`).
     */
    constexpr Listener(std::nullptr_t) noexcept {}

    /**
     * @brief Constructs a `Listener` with the specified shared pointer.
     *
     * @param listener The shared pointer to manage.
     */
    explicit Listener(std::shared_ptr<T> listener);

    /**
     * @brief Move constructor.
     *
     * Transfers ownership of the listener object from a temporary `Listener`.
     *
     * @param tmp The temporary `Listener` instance to move from.
     */
    Listener(Listener&& tmp) noexcept;

    /// @brief Copy constructor is disabled to prevent copying.
    Listener(const Listener&) = delete;

    /**
     * @brief Destructor.
     *
     * Ensures the listener is properly reset upon destruction.
     */
    ~Listener() { reset(); }

    /**
     * @brief Sets the shared pointer for the listener object.
     *
     * @param listener The shared pointer to set.
     */
    void set(std::shared_ptr<T> listener = {});

    /**
     * @brief Resets the listener object to null.
     */
    void reset() { set({}); }

    /**
     * @brief Checks if the listener object is empty.
     *
     * A listener is considered empty if the shared pointer is null.
     *
     * @return `true` if the listener is empty, otherwise `false`.
     */
    bool empty() const noexcept;
    
    /**
     * @brief Extracts and returns the listener object.
     *
     * @return An instance of the listener object.
     */
    std::shared_ptr<T> listener() const noexcept { return std::atomic_load(&_listener); }

    /**
     * @brief Invokes a method on the listener object with the specified arguments.
     *
     * @tparam Method The type of the method to invoke.
     * @tparam Args The types of the arguments to pass to the method.
     * @param method The method pointer to invoke on the listener.
     * @param args The arguments to pass to the method.
     */
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;

    /**
     * @brief Invokes a method on the listener object with the specified arguments, returning a result.
     *
     * @tparam R The return type of the method being invoked.
     * @tparam Method The type of the method to invoke.
     * @tparam Args The types of the arguments to pass to the method.
     * @param method The method pointer to invoke on the listener.
     * @param args The arguments to pass to the method.
     * @return The result of the method invocation.
     */
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const;

    /// @brief Copy assignment is disabled to prevent copying.
    Listener& operator=(const Listener&) = delete;

    /**
     * @brief Move assignment operator.
     *
     * Transfers ownership of the listener object from another `Listener`.
     *
     * @param tmp The temporary `Listener` instance to move from.
     * @return A reference to this `Listener`.
     */
    Listener& operator=(Listener&& tmp) noexcept;

    /**
     * @brief Assigns a new shared pointer as the listener object.
     *
     * @param listener The new shared pointer to assign.
     * @return A reference to this `Listener`.
     */
    Listener& operator=(std::shared_ptr<T> listener) noexcept;

    /**
     * @brief Checks if the listener is valid (non-empty).
     *
     * This operator allows the `Listener` to be used in a boolean context.
     *
     * @return `true` if the listener is valid, otherwise `false`.
     */
    explicit operator bool() const noexcept { return !empty(); }
private:
    /// @brief The shared pointer managing the listener object.
    std::shared_ptr<T> _listener;
};

template <typename T, bool ThreadSafe>
inline Listener<T, ThreadSafe>::Listener()
    : _listener(std::make_shared<Impl>())
{
}

template <typename T, bool ThreadSafe>
inline Listener<T, ThreadSafe>::Listener(std::nullptr_t)
    : _listener(std::make_shared<Impl>(nullptr))
{
}

template <typename T, bool ThreadSafe>
inline Listener<T, ThreadSafe>::Listener(T listener)
    : _listener(std::make_shared<Impl>(std::move(listener)))
{
}

template <typename T, bool ThreadSafe>
template <typename U>
inline void Listener<T, ThreadSafe>::set(U listener)
{
    const WriteLock lock(_listener->mutex());
    *_listener = std::move(listener);
}

template <typename T, bool ThreadSafe>
inline bool Listener<T, ThreadSafe>::empty() const noexcept
{
    const ReadLock lock(_listener->mutex());
    return Invoke<T>::empty(_listener->constRef());
}

template <typename T, bool ThreadSafe>
template <class Method, typename... Args>
inline void Listener<T, ThreadSafe>::
    invoke(const Method& method, Args&&... args) const
{
    const auto listener = _listener;
    const ReadLock lock(listener->mutex());
    Invoke<T>::make(listener->constRef(), method, std::forward<Args>(args)...);
}

template <typename T, bool ThreadSafe>
template <typename R, class Method, typename... Args>
inline R Listener<T, ThreadSafe>::invokeR(const Method& method, Args&&... args) const
{
    const auto listener = _listener;
    const ReadLock lock(listener->mutex());
    return Invoke<T, R>::makeR(listener->constRef(), method, std::forward<Args>(args)...);
}

template <typename T, bool ThreadSafe>
template <typename U>
inline Listener<T, ThreadSafe>& Listener<T, ThreadSafe>::operator = (U listener) noexcept
{
    set(std::move(listener));
    return *this;
}

// shared_ptr<> specialization
template <typename T, bool ThreadSafe>
inline Listener<std::shared_ptr<T>, ThreadSafe>::Listener(std::shared_ptr<T> listener)
    : _listener(std::move(listener))
{
}

template <typename T, bool ThreadSafe>
inline Listener<std::shared_ptr<T>, ThreadSafe>::Listener(Listener&& tmp) noexcept
{
    set(std::move(tmp._listener));
}

template <typename T, bool ThreadSafe>
inline void Listener<std::shared_ptr<T>, ThreadSafe>::set(std::shared_ptr<T> listener)
{
    if constexpr (ThreadSafe) {
        std::atomic_exchange(&_listener, std::move(listener));
    }
    else {
        _listener = std::move(listener);
    }
}

template <typename T, bool ThreadSafe>
inline bool Listener<std::shared_ptr<T>, ThreadSafe>::empty() const noexcept
{
    if constexpr (ThreadSafe) {
        return nullptr == std::atomic_load(&_listener);
    }
    return nullptr == _listener;
}

template <typename T, bool ThreadSafe>
template <class Method, typename... Args>
inline void Listener<std::shared_ptr<T>, ThreadSafe>::invoke(const Method& method,
                                                             Args&&... args) const
{
    if constexpr (ThreadSafe) {
        Invoke<std::shared_ptr<T>>::make(std::atomic_load(&_listener), method,
                                         std::forward<Args>(args)...);
    }
    else {
        Invoke<std::shared_ptr<T>>::make(&_listener, method,
                                         std::forward<Args>(args)...);
    }
}

template <typename T, bool ThreadSafe>
template <typename R, class Method, typename... Args>
inline R Listener<std::shared_ptr<T>, ThreadSafe>::invokeR(const Method& method, Args&&... args) const
{
    if constexpr (ThreadSafe) {
        return Invoke<std::shared_ptr<T>, R>::make(std::atomic_load(&_listener),
                                                   method, std::forward<Args>(args)...);
    }
    return Invoke<std::shared_ptr<T>, R>::make(_listener, method, std::forward<Args>(args)...);
}

template <typename T, bool ThreadSafe>
inline Listener<std::shared_ptr<T>, ThreadSafe>& Listener<std::shared_ptr<T>, ThreadSafe>::
    operator = (Listener&& tmp) noexcept
{
    if (&tmp != this) {
        if constexpr (ThreadSafe) {
            std::atomic_exchange(&_listener, std::move(tmp._listener));
        }
        else {
            _listener = std::move(tmp._listener);
        }
    }
    return *this;
}

template <typename T, bool ThreadSafe>
inline Listener<std::shared_ptr<T>, ThreadSafe>& Listener<std::shared_ptr<T>, ThreadSafe>::
    operator = (std::shared_ptr<T> listener) noexcept
{
    set(std::move(listener));
    return *this;
}

} // namespace Bricks
