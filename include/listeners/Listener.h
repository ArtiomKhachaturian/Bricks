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
#include "SafeObj.h"
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
 */
template <typename T>
class Listener
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
     * @brief Constructs a `Listener` with the specified listener object.
     *
     * @param listener The listener object to manage.
     */
    explicit Listener(T listener);

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
     * @brief Extracts and returns the listener object.
     *
     * @return An instance of the listener object.
     */
    T listener() const { return _listener(); }

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
    SafeObj<T, std::recursive_mutex> _listener = {};
};

/**
 * @brief Specialization of the `Listener` class for `std::shared_ptr` objects.
 *
 * This specialization provides lock-free management of listener objects stored
 * as `std::shared_ptr`. It supports operations similar to the base `Listener` class.
 *
 * @tparam T The type of the object managed by the shared pointer.
 */
template <typename T>
class Listener<std::shared_ptr<T>>
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

template <typename T>
inline Listener<T>::Listener(T listener)
    : _listener(std::move(listener))
{
}

template <typename T>
inline Listener<T>::Listener(Listener&& tmp) noexcept
{
    _listener = tmp._listener.take();
}

template <typename T>
template <typename U>
inline void Listener<T>::set(U listener)
{
    LOCK_WRITE_SAFE_OBJ(_listener);
    _listener = std::move(listener);
}

template <typename T>
inline bool Listener<T>::empty() const noexcept
{
    LOCK_READ_SAFE_OBJ(_listener);
    return Invoke<T>::empty(_listener.constRef());
}

template <typename T>
template <class Method, typename... Args>
inline void Listener<T>::invoke(const Method& method, Args&&... args) const
{
    LOCK_READ_SAFE_OBJ(_listener);
    Invoke<T>::make(_listener.constRef(), method, std::forward<Args>(args)...);
}

template <typename T>
template <typename R, class Method, typename... Args>
inline R Listener<T>::invokeR(const Method& method, Args&&... args) const
{
    LOCK_READ_SAFE_OBJ(_listener);
    return Invoke<T, R>::make(_listener.constRef(), method, std::forward<Args>(args)...);
}

template <typename T>
inline Listener<T>& Listener<T>::operator = (Listener&& tmp) noexcept
{
    if (&tmp != this) {
        set(tmp._listener.take());
    }
    return *this;
}

template <typename T>
template <typename U>
inline Listener<T>& Listener<T>::operator = (U listener) noexcept
{
    set(std::move(listener));
    return *this;
}

template <typename T>
inline Listener<std::shared_ptr<T>>::Listener(std::shared_ptr<T> listener)
    : _listener(std::move(listener))
{
}

template <typename T>
inline Listener<std::shared_ptr<T>>::Listener(Listener&& tmp) noexcept
{
    set(std::move(tmp._listener));
}

template <typename T>
inline void Listener<std::shared_ptr<T>>::set(std::shared_ptr<T> listener)
{
    std::atomic_exchange(&_listener, std::move(listener));
}

template <typename T>
inline bool Listener<std::shared_ptr<T>>::empty() const noexcept
{
    return nullptr == std::atomic_load(&_listener);
}

template <typename T>
template <class Method, typename... Args>
inline void Listener<std::shared_ptr<T>>::invoke(const Method& method,
                                                 Args&&... args) const
{
    Invoke<std::shared_ptr<T>>::make(std::atomic_load(&_listener),
                                     method,
                                     std::forward<Args>(args)...);
}

template <typename T>
template <typename R, class Method, typename... Args>
inline R Listener<std::shared_ptr<T>>::invokeR(const Method& method, Args&&... args) const
{
    return Invoke<std::shared_ptr<T>, R>::make(std::atomic_load(&_listener),
                                               method, std::forward<Args>(args)...);
}

template <typename T>
inline Listener<std::shared_ptr<T>>& Listener<std::shared_ptr<T>>::
    operator = (Listener&& tmp) noexcept
{
    if (&tmp != this) {
        std::atomic_exchange(&_listener, std::move(tmp._listener));
    }
    return *this;
}

template <typename T>
inline Listener<std::shared_ptr<T>>& Listener<std::shared_ptr<T>>::
    operator = (std::shared_ptr<T> listener) noexcept
{
    set(std::move(listener));
    return *this;
}

} // namespace Bricks
