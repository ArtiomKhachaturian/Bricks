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
#include "ListenerStorage.h"

namespace Bricks
{

/**
 * @brief A threadsafe wrapper for managing listener objects with atomic or mutex-based locking.
 *
 * The `Listener` class provides an interface for managing listener objects in multithreaded
 * environments. It ensures safe access with locks while allowing flexibility through
 * customizable operations, such as invoking methods on the listener or resetting it.
 *
 * @tparam T The type/class of the listener object being managed.
 * @tparam ThreadSafe   If true, enables thread-safe operations using a atomics or mutex (default: true).
 *
 * Note: the mutex is chosen for std::weak_ptr or raw pointer.
 */
// T is shared_ptr/unique_ptr/weak_ptr/T*
template <typename T, bool ThreadSafe = true> class Listener {
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
    Listener(std::nullptr_t) : _s{} {}

    /**
     * @brief Constructs a `Listener` with the specified listener object.
     *
     * @param listener The listener object to manage.
     */
    explicit Listener(T listener) : _s{std::move(listener)} {}

    Listener(const Listener& src) : _s{src._s} {}
    
    Listener(Listener&& tmp) noexcept : _s{std::move(tmp._s)} {}
    
    ~Listener() { reset(); }
    
    Listener& operator = (const Listener& src) {
        if (&src != this) {
            _s = src._s;
        }
        return *this;
    }
    
    Listener& operator = (Listener&& tmp) noexcept {
        if (&tmp != this) {
            _s = std::move(tmp._s);
        }
        return *this;
    }
    
    /**
     * @brief Sets the listener object.
     *
     * @tparam U The type of the listener object (defaults to `T`).
     * @param listener The listener object to set.
     */
    template <typename U = T>
    void set(U listener = {}) {
        _s.set(std::move(listener));
    }
    
    /**
     * @brief Resets the listener object to null.
     */
    void reset() { set(T{}); }
    
    /**
     * @brief Checks if the listener object is empty.
     *
     * A listener is considered empty if it has not been set or has been reset to null.
     *
     * @return `true` if the listener is empty, otherwise `false`.
     */
    bool empty() const noexcept { return _s.empty(); }

    /**
     * @brief Invokes a method on the listener object with the specified arguments.
     *
     * @tparam Method The type of the method to invoke.
     * @tparam Args The types of the arguments to pass to the method.
     * @param method The method pointer to invoke on the listener.
     * @param args The arguments to pass to the method.
     */
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        return _s.invoke(method, std::forward<Args>(args)...);
    }
    
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
    R invokeR(const Method& method, Args&&... args) const {
        return _s.template invokeR<R>(method, std::forward<Args>(args)...);
    }

    /**
     * @brief Assigns a new listener object.
     *
     * @tparam U The type of the listener object (defaults to `T`).
     * @param listener The new listener object to assign.
     * @return A reference to this `Listener`.
     */
    template <typename U = T>
    Listener& operator=(U listener) noexcept {
        set(std::move(listener));
        return *this;
    }

    /**
     * @brief Checks if the listener is valid (non-empty).
     *
     * This operator allows the `Listener` to be used in a boolean context.
     *
     * @return `true` if the listener is valid, otherwise `false`.
     */
    explicit operator bool() const noexcept { return !empty(); }
private:
    internal::ListenerStorage<T, ThreadSafe> _s;
};

} // namespace Bricks
