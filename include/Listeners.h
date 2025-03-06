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
#include "ProtectedObj.h"
#include <algorithm>
#include <memory>
#include <vector>

template<bool ThreadSafe>
struct ListenersMutexSelector
{
    using MutexType = std::recursive_mutex;
};

template<>
struct ListenersMutexSelector<false>
{
    class StubMutex
    {
    public:
        StubMutex() = default;
        void lock() {}
        bool try_lock() { return true; }
        void unlock() {}
    };
    using MutexType = StubMutex;
};

// Listeners array wrapper (mutex lock by default),
// supported all most known types like raw pointers,
// std::shared_ptr<>, std::weak_ptr<>.
// Note: std::unique_ptr<> doesn't supported
template<class TListener, bool ThreadSafe = true>
class Listeners
{
    using MutexType = typename ListenersMutexSelector<ThreadSafe>::MutexType;
public:
    Listeners();
    Listeners(Listeners&& tmp) noexcept;
    Listeners(const Listeners& other);
    ~Listeners() { clear(); }
    bool add(const TListener& listener);
    bool remove(const TListener& listener);
    void clear();
    bool empty() const noexcept;
    size_t size() const noexcept;
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;
    Listeners& operator = (const Listeners& other);
    Listeners& operator = (Listeners&& tmp) noexcept;
private:
    ProtectedObj<std::vector<TListener>, MutexType> _listeners;
};

template<class TListener, bool ThreadSafe>
inline Listeners<TListener, ThreadSafe>::Listeners()
{
    _listeners->reserve(1U);
}

template<class TListener, bool ThreadSafe>
inline Listeners<TListener, ThreadSafe>::Listeners(const Listeners& other)
{
    LOCK_READ_PROTECTED_OBJ(other._listeners);
    _listeners = other._listeners.constRef();
}

template<class TListener, bool ThreadSafe>
inline Listeners<TListener, ThreadSafe>::Listeners(Listeners&& tmp) noexcept
{
    _listeners = tmp._listeners.take();
}

template<class TListener, bool ThreadSafe>
inline bool Listeners<TListener, ThreadSafe>::add(const TListener& listener)
{
    if (listener) {
        LOCK_WRITE_PROTECTED_OBJ(_listeners);
        const auto it = std::find(_listeners->begin(), _listeners->end(), listener);
        if (it == _listeners->end()) {
            _listeners->push_back(listener);
            return true;
        }
    }
    return false;
}

template<class TListener, bool ThreadSafe>
inline bool Listeners<TListener, ThreadSafe>::remove(const TListener& listener)
{
    if (listener) {
        LOCK_WRITE_PROTECTED_OBJ(_listeners);
        const auto it = std::find(_listeners->begin(), _listeners->end(), listener);
        if (it != _listeners->end()) {
            _listeners->erase(it);
            return true;
        }
    }
    return false;
}

template<class TListener, bool ThreadSafe>
inline void Listeners<TListener, ThreadSafe>::clear()
{
    LOCK_WRITE_PROTECTED_OBJ(_listeners);
    _listeners->clear();
}

template<class TListener, bool ThreadSafe>
inline bool Listeners<TListener, ThreadSafe>::empty() const noexcept
{
    LOCK_READ_PROTECTED_OBJ(_listeners);
    return _listeners->empty();
}

template<class TListener, bool ThreadSafe>
inline size_t Listeners<TListener, ThreadSafe>::size() const noexcept
{
    LOCK_READ_PROTECTED_OBJ(_listeners);
    return _listeners->size();
}

template<class TListener, bool ThreadSafe>
template <class Method, typename... Args>
inline void Listeners<TListener, ThreadSafe>::invoke(const Method& method,
                                                     Args&&... args) const
{
    LOCK_READ_PROTECTED_OBJ(_listeners);
    const auto& listeners = _listeners.constRef();
    if (!listeners.empty()) {
        size_t i = 0UL;
        do {
            const size_t size = listeners.size();
            if (i < size) {
                Invoke<TListener>::make(listeners[i], method, std::forward<Args>(args)...);
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

template<class TListener, bool ThreadSafe>
inline Listeners<TListener, ThreadSafe>& Listeners<TListener, ThreadSafe>::
    operator = (const Listeners& other)
{
    if (&other != this) {
        LOCK_WRITE_PROTECTED_OBJ(_listeners);
        LOCK_READ_PROTECTED_OBJ(other._listeners);
        _listeners = other._listeners.constRef();
    }
    return *this;
}

template<class TListener, bool ThreadSafe>
inline Listeners<TListener, ThreadSafe>& Listeners<TListener, ThreadSafe>::
    operator = (Listeners&& tmp) noexcept
{
    if (&tmp != this) {
        LOCK_WRITE_PROTECTED_OBJ(_listeners);
        _listeners = tmp._listeners.take();
    }
    return *this;
}
