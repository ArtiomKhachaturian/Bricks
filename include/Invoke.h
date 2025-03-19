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

namespace Bricks
{

/**
 * @brief A helper class (in mostly for internal usage) for invoking methods on listener objects.
 *
 * The `Invoke` class provides a unified interface to call methods on listener objects
 * of various types, such as raw pointers, shared pointers, and weak pointers.
 * It ensures safe and flexible invocation with the provided arguments.
 *
 * @tparam TListener The type of the listener object (e.g., raw pointer, std::shared_ptr).
 * @tparam TResult The return type of the invoked method (defaults to `void`).
 */
template<class TListener, typename TResult = void>
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
    static TResult make(const TListener& listener,
                        const Method& method,
                        Args&&... args) {
        if (listener) {
            return ((*listener).*method)(std::forward<Args>(args)...);
        }
        return TResult();
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
template<typename T, typename TResult>
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
     * @param listenerRef The weak pointer referencing the listener object.
     * @param method The method pointer to be invoked on the listener.
     * @param args The arguments to pass to the method.
     * @return The result of the invoked method, or a default-constructed `TResult` if the lock fails.
     */
    template <class Method, typename... Args>
    static TResult make(const std::weak_ptr<T>& listenerRef,
                        const Method& method,
                        Args&&... args) {
        return Forward::make(listenerRef.lock(), method, std::forward<Args>(args)...);
    }
};

} // namespace Bricks
