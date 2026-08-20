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
#include "Invoke.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
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
 * but `std::unique_ptr` doesn't supported for thread-safe version.
 *
 * @tparam T The type of the listener objects.
 * @tparam ThreadSafe If true, enables thread-safe operations using a mutex (default: true).
 *                    If false, no mutex locking is performed (via `StubMutex`).
 */
template <class T, bool ThreadSafe = true> class Listeners;

template <class T>
class Listeners<T, true>
{
    using Container = std::vector<std::pair<uint64_t, T>>;
public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `Listeners` instance.
     */
    Listeners() : _snapshot(std::make_shared<Container>()) {}
    
    /// @brief Copy & move assignment are disabled.
    Listeners(Listeners&&) noexcept = delete;
    Listeners(const Listeners&) = delete;
    Listeners& operator=(const Listeners&) = delete;
    Listeners& operator=(Listeners&&) noexcept = delete;

    /**
     * @brief Destructor.
     *
     * Clears the list of listeners upon destruction.
     */
    ~Listeners() { clear(); }

    /**
     * @brief Adds a listener to the list.
     *
     * Adds the given listener to the collection.
     *
     * @param listener The listener object to add.
     * @return zero if failed, otherwise - ID of added listener.
     */
    uint64_t add(T listener, bool* wasAdded1st = nullptr) {
        uint64_t id = 0ULL;
        if (!Invoke<T>::empty(listener)) {
            const std::lock_guard guard(_m);
            id = ++_next;
            const auto old_snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_relaxed);
            auto new_snapshot = std::make_shared<Container>(*old_snapshot); // make deep copy
            new_snapshot->emplace_back(std::make_pair(id, std::move(listener)));
            if (wasAdded1st && 1 == new_snapshot->size()) {
                *wasAdded1st = true;
            }
            std::atomic_store_explicit(&_snapshot, std::move(new_snapshot), std::memory_order_release);
        }
        return id;
    }

    /**
     * @brief Removes a listener from the list.
     *
     * Removes the specified listener from the collection.
     *
     * @param id listener's id  to remove.
     * @return true if listener has been removed.
     */
    bool remove(uint64_t id, bool* wasRemovedLast = nullptr) {
        bool found = false;
        if (id) {
            const std::lock_guard guard(_m);
            const auto old_snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_relaxed);
            found = std::any_of(old_snapshot->begin(), old_snapshot->end(), [id](const auto& l) {
                return l.first == id;
            });
            if (found) {
                auto new_snapshot = std::make_shared<Container>();
                new_snapshot->reserve(old_snapshot->size() - 1);
                std::copy_if(old_snapshot->begin(), old_snapshot->end(), std::back_inserter(*new_snapshot),
                             [id](const auto& l) { return l.first != id; });
                if (wasRemovedLast) {
                    *wasRemovedLast = new_snapshot->empty();
                }
                std::atomic_store_explicit(&_snapshot, std::move(new_snapshot), std::memory_order_release);
            }
        }
        return found;
    }
    
    /**
     * @brief Checks if the given listener is present in the list.
     *
     *
     * @param id listener's id  to check.
     * @return `true` if the list contains such listener, otherwise `false`.
     */
    bool contains(uint64_t id) const {
        if (id) {
            const auto snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_acquire);
            const auto it = std::find_if(snapshot->begin(), snapshot->end(), [id](const auto& l) {
                return l.first == id;
            });
            return it != snapshot->end();
        }
        return false;
    }

    /**
     * @brief Clears all listeners from the list.
     *
     * Removes all listeners from the collection.
     *
     * @return `true` if the list was not empty, otherwise `false`.
     */
    bool clear() {
        const std::lock_guard guard(_m);
        const auto old_snapshot = std::atomic_exchange_explicit(&_snapshot,
                                                                std::make_shared<Container>(),
                                                                std::memory_order_release);
        return !old_snapshot->empty();
    }

    /**
     * @brief Checks if the list of listeners is empty.
     *
     * @return `true` if the list is empty, otherwise `false`.
     */
    bool empty() const noexcept {
        const auto snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_acquire);
        return snapshot->empty();
    }

    /**
     * @brief Gets the number of listeners in the list.
     *
     * @return The number of listeners in the collection.
     */
    size_t size() const noexcept {
        const auto snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_acquire);
        return snapshot->size();
    }

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
    void invoke(const Method& method, Args&&... args) const {
        const auto snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_acquire);
        std::for_each(snapshot->begin(), snapshot->end(), [&](const auto& l) {
            Invoke<T>::make(l.second, method, std::forward<Args>(args)...);
        });
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
    void apply(const Functor& functor) const {
        const auto snapshot = std::atomic_load_explicit(&_snapshot, std::memory_order_acquire);
        std::for_each(snapshot->begin(), snapshot->end(), [&functor](const auto& l) {
            Invoke<T>::apply(l.second, functor);
        });
    }
    
    /**
     * @brief Checks if the listeners is valid (non-empty).
     *
     * This operator allows the `Listeners` to be used in a boolean context.
     *
     * @return `true` if the listener is valid, otherwise `false`.
     */
    explicit operator bool() const noexcept { return !empty(); }
private:
    std::mutex _m; // for snapshot modifications only
    uint64_t _next = 0ULL;
    std::shared_ptr<Container> _snapshot;
};

template <class T>
class Listeners<T, false> {
    using Container = std::vector<std::pair<uint64_t, T>>;
public:
    Listeners() = default;
    Listeners(Listeners&& tmp) noexcept
        : _next(tmp._next)
        , _snapshot(std::move(tmp._snapshot)) {
        tmp._next = 0ULL;
    }
    /// @brief Copy assignment is disabled.
    Listeners(const Listeners&) = delete;
    Listeners& operator=(const Listeners&) = delete;
    Listeners& operator=(Listeners&& tmp) noexcept {
        if (&tmp != this) {
            _next = tmp._next;
            _snapshot = std::move(tmp._snapshot);
            tmp._next = 0ULL;
        }
        return *this;
    }
    ~Listeners() { clear(); }

    uint64_t add(T listener, bool* wasAdded1st = nullptr) {
        uint64_t id = 0ULL;
        if (!Invoke<T>::empty(listener)) {
            id = ++_next;
            _snapshot.push_back(std::make_pair(id, std::move(listener)));
            if (wasAdded1st && 1 == _snapshot.size()) {
                *wasAdded1st = true;
            }
        }
        return id;
    }

    bool remove(uint64_t id, bool* wasRemovedLast = nullptr) {
        bool found = false;
        if (id) {
            for (auto it = _snapshot.begin(); it != _snapshot.end(); ++it) {
                if (it->first == id) {
                    _snapshot.erase(it);
                    found = true;
                    break;
                }
            }
            if (found && wasRemovedLast) {
                *wasRemovedLast = _snapshot.empty();
            }
        }
        return found;
    }
   
    bool contains(uint64_t id) const {
        if (id) {
            const auto it = std::find_if(_snapshot.begin(), _snapshot.end(), [id](const auto& l) {
                return l.first == id;
            });
            return it != _snapshot.end();
        }
        return false;
    }

    bool clear() {
        const auto empty = _snapshot.empty();
        _snapshot.clear();
        return !empty;
    }

    bool empty() const noexcept { return _snapshot.empty(); }

    size_t size() const noexcept { return _snapshot.size(); }

    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        std::for_each(_snapshot.begin(), _snapshot.end(), [&](const auto& l) {
            Invoke<T>::make(l.second, method, std::forward<Args>(args)...);
        });
    }

    template <class Functor>
    void apply(const Functor& functor) const {
        std::for_each(_snapshot.begin(), _snapshot.end(), [&functor](const auto& l) {
            Invoke<T>::apply(l.second, functor);
        });
    }
    
    explicit operator bool() const noexcept { return !empty(); }
private:
    uint64_t _next = 0ULL;
    Container _snapshot;
    
};

} // namespace Bricks
