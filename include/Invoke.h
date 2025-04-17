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
#pragma once // Invoke.h
#include <memory>
#include <vector>

namespace Bricks
{

/**
 * @brief A helper class (in mostly for internal usage) for invoking methods on listener objects.
 *
 * The `Invoke` class provides a unified interface to call methods on listener objects
 * of various types, such as raw pointers, shared pointers, and weak pointers.
 * It ensures safe and flexible invocation with the provided arguments.
 *
 * @tparam T The type of the listener object (e.g., raw pointer, std::shared_ptr).
 * @tparam TResult The return type of the invoked method (defaults to `void`).
 */
template <class T, typename TResult = void>
class Invoke
{
public:
    /**
     * @brief Invokes a method on the listener object with the specified arguments.
     *
     * If the listener is valid (non-null), the specified method is invoked with the provided arguments.
     * Otherwise, a default-constructed `TResult` is returned.
     *
     * @tparam Method The type of the method to be invoked.
     * @tparam Args The types of the arguments to be passed to the method.
     * @param listener The listener object on which the method will be invoked.
     * @param method The method pointer to be invoked on the listener.
     * @param args The arguments to pass to the method.
     * @return The result of the invoked method, or a default-constructed `TResult` if the listener is invalid.
     */
    template <class Method, typename... Args>
    static TResult make(const T& listener, const Method& method, Args&&... args) {
        if (listener) {
            return ((*listener).*method)(std::forward<Args>(args)...);
        }
        return TResult();
    }
    
    /**
     * @brief Invokes a method on the listener objects (collection) with the specified arguments.
     *
     *
     * @tparam Method The type of the method to be invoked.
     * @tparam Args The types of the arguments to be passed to the method.
     * @param listener The listener object on which the method will be invoked.
     * @param method The method pointer to be invoked on the listener.
     * @param args The arguments to pass to the method.
     */
    template <class Method, typename... Args>
    static void make(const std::vector<T>& listeners,
                     const Method& method, Args&&... args) {
        if (!listeners.empty()) {
            size_t i = 0UL;
            do {
                const size_t size = listeners.size();
                if (i < size) {
                    make(listeners[i], method, std::forward<Args>(args)...);
                    if (listeners.size() >= size) {
                        ++i;
                    }
                }
                else {
                    break;
                }
            }
            while(true);
        }
    }
    
    /**
     * @brief Applies a given functor to each listener in the list.
     *
     * This method iterates over all listeners in the list and invokes
     * the provided functor for each listener.
     *
     * @tparam Functor A callable type that defines operator()(listener) or similar.
     * @param functor The functor to apply to each listener. It must be capable of
     *                accepting a single argument of the type stored in the list.
     */
    template <class Functor>
    static void apply(const std::vector<T>& listeners, const Functor& functor) {
        if (!listeners.empty()) {
            size_t i = 0UL;
            do {
                const size_t size = listeners.size();
                if (i < size) {
                    functor(listeners[i]);
                    if (listeners.size() >= size) {
                        ++i;
                    }
                }
                else {
                    break;
                }
            }
            while(true);
        }
    }
    
    /**
     * @brief Verifies if the referenced listener object is null.
     *
     * This method checks whether the provided listener is uninitialized
     * or does not reference a valid object. It can be used to ensure
     * that the listener is valid before invoking any methods on it.
     *
     * @param listener The listener object to verify.
     * @return `true` if the listener is null; otherwise, `false`.
     */
    static bool empty(const T& listener) {
        return nullptr == listener;
    }
};

/**
 * @brief A specialization of the `Invoke` class for `std::weak_ptr` listeners.
 *
 * This specialization provides safe invocation for listeners stored as `std::weak_ptr`.
 * It attempts to lock the weak pointer and invokes the method if the lock succeeds.
 *
 * @tparam T The type of the object managed by the weak pointer.
 * @tparam TResult The return type of the invoked method (defaults to `void`).
 */
template <typename T, typename TResult>
class Invoke<std::weak_ptr<T>, TResult>
{
    /// @brief Helper alias for invoking methods on `std::shared_ptr` instances.
    using Forward = Invoke<std::shared_ptr<T>, TResult>;

public:
    /**
     * @brief Invokes a method on the listener object referenced by the weak pointer.
     *
     * The weak pointer is locked to obtain a `std::shared_ptr`. If the lock succeeds, the
     * method is invoked on the shared object. Otherwise, a default-constructed `TResult` is returned.
     *
     * @tparam Method The type of the method to be invoked.
     * @tparam Args The types of the arguments to be passed to the method.
     * @param listener The weak pointer referencing the listener object.
     * @param method The method pointer to be invoked on the listener.
     * @param args The arguments to pass to the method.
     * @return The result of the invoked method, or a default-constructed `TResult` if the lock fails.
     */
    template <class Method, typename... Args>
    static TResult make(const std::weak_ptr<T>& listener,
                        const Method& method,
                        Args&&... args) {
        return Forward::make(listener.lock(), method, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Invokes a method on the listener objects (collection) referenced by the weak pointer.
     *
     * The weak pointer is locked to obtain a `std::shared_ptr`. If the lock succeeds, the
     * method is invoked on the shared object.
     *
     * @tparam Method The type of the method to be invoked.
     * @tparam Args The types of the arguments to be passed to the method.
     * @param listener The weak pointer referencing the listener object.
     * @param method The method pointer to be invoked on the listener.
     * @param args The arguments to pass to the method.
     */
    template <class Method, typename... Args>
    static void make(const std::vector<std::weak_ptr<T>>& listeners,
                     const Method& method, Args&&... args) {
        if (!listeners.empty()) {
            size_t i = 0UL;
            do {
                const size_t size = listeners.size();
                if (i < size) {
                    Forward::make(listeners[i].lock(), method, std::forward<Args>(args)...);
                    if (listeners.size() >= size) {
                        ++i;
                    }
                }
                else {
                    break;
                }
            }
            while(true);
        }
    }
    
    /**
     * @brief Applies a given functor to each listener in the list.
     *
     * This method iterates over all listeners in the list and invokes
     * the provided functor for each listener.
     *
     * @tparam Functor A callable type that defines operator()(listener) or similar.
     * @param functor The functor to apply to each listener. It must be capable of
     *                accepting a single argument of the type stored in the list.
     */
    template <class Functor>
    static void apply(const std::vector<std::weak_ptr<T>>& listeners, const Functor& functor) {
        if (!listeners.empty()) {
            size_t i = 0UL;
            do {
                const size_t size = listeners.size();
                if (i < size) {
                    if (const auto listener = listeners[i].lock()) {
                        functor(listener);
                    }
                    if (listeners.size() >= size) {
                        ++i;
                    }
                }
                else {
                    break;
                }
            }
            while(true);
        }
    }
    
    /**
     * @brief Checks whether the object referenced by the weak pointer has been deleted.
     *
     * This method verifies whether the weak pointer refers to an expired object, indicating
     * that the referenced object has been destroyed or no longer exists. It is particularly
     * useful for ensuring the validity of a `std::weak_ptr` before attempting to lock it
     * or invoke any methods on the referenced object.
     *
     * @param listener The weak pointer referencing the listener object.
     * @return `true` if the referenced object has been deleted (expired), otherwise `false`.
     */
    static bool empty(const std::weak_ptr<T>& listener) {
        return listener.expired();
    }
};

} // namespace Bricks
