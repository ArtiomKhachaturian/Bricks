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
#pragma once // Listeners.h
#include "AddResult.h"
#include "RemoveResult.h"
#include "Invoke.h"
#include "ListenersMutexSelector.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

namespace Bricks
{

/**
 * @brief A thread-safe wrapper for managing a list of listeners.
 *
 * The `Listeners` class provides a convenient interface for managing a collection of listeners.
 * It supports thread-safe operations by default, but can be configured for non-thread-safe usage.
 * Common listener types such as raw pointers, `std::shared_ptr`, and `std::weak_ptr` are supported,
 * but `std::unique_ptr` doesn't supported.
 *
 * @tparam T The type of the listener objects.
 * @tparam ThreadSafe If true, enables thread-safe operations using a mutex (default: true).
 *                    If false, no mutex locking is performed (via `StubMutex`).
 */
template <class T, bool ThreadSafe = true>
class Listeners
{
    /// @brief Defines the type of mutex based on the thread-safety configuration.
    using MutexType = typename ListenersMutexSelector<ThreadSafe>::MutexType;
    using ReadLock = typename MutexTraits<MutexType>::ReadLock;
    using WriteLock = typename MutexTraits<MutexType>::WriteLock;
    using Impl = SafeObj<std::vector<T>, MutexType>;
public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `Listeners` instance.
     */
    Listeners();
    
    /// @brief Copy & move assignment are disabled.
    Listeners(Listeners&&) = delete;
    Listeners(const Listeners&) = delete;
    Listeners& operator=(const Listeners&) = delete;
    Listeners& operator=(Listeners&&) = delete;

    /**
     * @brief Destructor.
     *
     * Clears the list of listeners upon destruction.
     */
    ~Listeners() { clear(); }

    /**
     * @brief Adds a listener to the list.
     *
     * Adds the given listener to the collection. If the listener already exists,
     * it will not be added again.
     *
     * @param listener The listener object to add.
     * @return The result of the add operation (e.g., `AddResult::Ok`, `AddResult::Duplicate`).
     */
    AddResult add(const T& listener);

    /**
     * @brief Removes a listener from the list.
     *
     * Removes the specified listener from the collection.
     *
     * @param listener The listener object to remove.
     * @return The result of the removal operation (e.g., `RemoveResult::Ok`, `RemoveResult::NullInput`).
     */
    RemoveResult remove(const T& listener);
    
    /**
     * @brief Checks if the given listener is present in the list.
     *
     *
     * @param listener The listener object to check.
     * @return `true` if the list contains such listener, otherwise `false`.
     */
    bool contains(const T& listener) const;

    /**
     * @brief Clears all listeners from the list.
     *
     * Removes all listeners from the collection.
     *
     * @return `true` if the list was not empty, otherwise `false`.
     */
    bool clear();

    /**
     * @brief Checks if the list of listeners is empty.
     *
     * @return `true` if the list is empty, otherwise `false`.
     */
    bool empty() const noexcept;

    /**
     * @brief Gets the number of listeners in the list.
     *
     * @return The number of listeners in the collection.
     */
    size_t size() const noexcept;

    /**
     * @brief Invokes a method on all listeners in the list with the specified arguments.
     *
     * This function iterates over the listeners and invokes the given method with the provided arguments.
     *
     * @tparam Method The type of the method to invoke.
     * @tparam Args The types of the arguments to pass to the method.
     * @param method The method pointer to invoke on each listener.
     * @param args The arguments to pass to the method.
     */
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;
    
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
    void apply(const Functor& functor) const;
    
    /**
     * @brief Checks if the listeners is valid (non-empty).
     *
     * This operator allows the `Listeners` to be used in a boolean context.
     *
     * @return `true` if the listener is valid, otherwise `false`.
     */
    explicit operator bool() const noexcept { return !empty(); }
private:
    /// @brief A thread-safe wrapper around the vector of listener objects.
    const std::shared_ptr<Impl> _listeners;
};

template <class T, bool ThreadSafe>
inline Listeners<T, ThreadSafe>::Listeners()
    : _listeners(std::make_shared<Impl>())
{
    (*_listeners)->reserve(1U);
}

template <class T, bool ThreadSafe>
inline AddResult Listeners<T, ThreadSafe>::add(const T& listener)
{
    if (listener) {
        const WriteLock lock(_listeners->mutex());
        auto& listeners = *_listeners;
        const auto it = std::find(listeners->begin(), listeners->end(), listener);
        if (it == listeners->end()) {
            listeners->push_back(listener);
            return 1U == listeners->size() ? AddResult::OkFirst : AddResult::Ok;
        }
        return AddResult::Duplicate;
    }
    return AddResult::NullInput;
}

template <class T, bool ThreadSafe>
inline RemoveResult Listeners<T, ThreadSafe>::remove(const T& listener)
{
    if (listener) {
        const WriteLock lock(_listeners->mutex());
        auto& listeners = *_listeners;
        const auto it = std::find(listeners->begin(), listeners->end(), listener);
        if (it != listeners->end()) {
            listeners->erase(it);
            return listeners->empty() ? RemoveResult::OkLast : RemoveResult::Ok;
        }
    }
    return RemoveResult::NullInput;
}

template <class T, bool ThreadSafe>
inline bool Listeners<T, ThreadSafe>::contains(const T& listener) const
{
    if (listener) {
        const ReadLock lock(_listeners->mutex());
        const auto& listeners = *_listeners;
        const auto it = std::find(listeners->begin(), listeners->end(), listener);
        return it != listeners->end();
    }
    return false;
}

template <class T, bool ThreadSafe>
inline bool Listeners<T, ThreadSafe>::clear()
{
    const WriteLock lock(_listeners->mutex());
    auto& listeners = *_listeners;
    if (!listeners->empty()) {
        listeners->clear();
        return true;
    }
    return false;
}

template <class T, bool ThreadSafe>
inline bool Listeners<T, ThreadSafe>::empty() const noexcept
{
    const ReadLock lock(_listeners->mutex());
    return (*_listeners)->empty();
}

template <class T, bool ThreadSafe>
inline size_t Listeners<T, ThreadSafe>::size() const noexcept
{
    const ReadLock lock(_listeners->mutex());
    return (*_listeners)->size();
}

template <class T, bool ThreadSafe>
template <class Method, typename... Args>
inline void Listeners<T, ThreadSafe>::invoke(const Method& method, Args&&... args) const
{
    const auto listeners = _listeners;
    const ReadLock lock(listeners->mutex());
    Invoke<T>::make(listeners->constRef(), method, std::forward<Args>(args)...);
}

template <class T, bool ThreadSafe>
template <class Functor>
inline void Listeners<T, ThreadSafe>::apply(const Functor& functor) const
{
    const auto listeners = _listeners;
    const ReadLock lock(listeners->mutex());
    Invoke<T>::apply(listeners->constRef(), functor);
}

} // namespace Bricks
