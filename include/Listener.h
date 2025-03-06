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

// threadsafe listener wrapper (mutex lock)
template<class TListener>
class Listener
{
public:
    Listener() = default;
    constexpr Listener(std::nullptr_t) noexcept {}
    explicit Listener(TListener listener);
    Listener(Listener&& tmp) noexcept;
    Listener(const Listener&) = delete;
    ~Listener() { reset(); }
    void set(TListener listener = {});
    void reset() { set({}); }
    bool empty() const noexcept;
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;
    Listener& operator = (const Listener&) = delete;
    Listener& operator = (Listener&& tmp) noexcept;
    Listener& operator = (TListener listener) noexcept;
private:
    SafeObj<TListener, std::recursive_mutex> _listener;
};

// lock-free specialization for std::shared_ptr<>
template<typename T>
class Listener<std::shared_ptr<T>>
{
public:
    Listener() = default;
    constexpr Listener(std::nullptr_t) noexcept {}
    explicit Listener(std::shared_ptr<T> listener);
    Listener(Listener&& tmp) noexcept;
    Listener(const Listener&) = delete;
    ~Listener() { reset(); }
    void set(std::shared_ptr<T> listener = {});
    void reset() { set({}); }
    bool empty() const noexcept;
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;
    Listener& operator = (const Listener&) = delete;
    Listener& operator = (Listener&& tmp) noexcept;
    Listener& operator = (std::shared_ptr<T> listener) noexcept;
private:
    std::shared_ptr<T> _listener;
};


template<class TListener>
inline Listener<TListener>::Listener(TListener listener)
    : _listener(std::move(listener))
{
}

template<class TListener>
inline Listener<TListener>::Listener(Listener&& tmp) noexcept
{
    _listener = tmp._listener.take();
}

template<class TListener>
inline void Listener<TListener>::set(TListener listener)
{
    LOCK_WRITE_SAFE_OBJ(_listener);
    _listener = std::move(listener);
}

template<class TListener>
inline bool Listener<TListener>::empty() const noexcept
{
    LOCK_READ_SAFE_OBJ(_listener);
    return nullptr == _listener.constRef();
}

template<class TListener>
template <class Method, typename... Args>
inline void Listener<TListener>::invoke(const Method& method, Args&&... args) const
{
    LOCK_READ_SAFE_OBJ(_listener);
    Invoke<TListener>::make(_listener.constRef(), method, std::forward<Args>(args)...);
}

template<class TListener>
inline Listener<TListener>& Listener<TListener>::operator = (Listener&& tmp) noexcept
{
    if (&tmp != this) {
        set(tmp._listener.take());
    }
    return *this;
}

template<class TListener>
inline Listener<TListener>& Listener<TListener>::operator = (TListener listener) noexcept
{
    set(std::move(listener));
    return *this;
}

template<typename T>
inline Listener<std::shared_ptr<T>>::Listener(std::shared_ptr<T> listener)
    : _listener(std::move(listener))
{
}

template<typename T>
inline Listener<std::shared_ptr<T>>::Listener(Listener&& tmp) noexcept
{
    set(std::move(tmp._listener));
}

template<typename T>
inline void Listener<std::shared_ptr<T>>::set(std::shared_ptr<T> listener)
{
    std::atomic_exchange(&_listener, std::move(listener));
}

template<typename T>
inline bool Listener<std::shared_ptr<T>>::empty() const noexcept
{
    return nullptr == std::atomic_load(&_listener);
}

template<typename T>
template <class Method, typename... Args>
inline void Listener<std::shared_ptr<T>>::invoke(const Method& method,
                                                 Args&&... args) const
{
    Invoke<std::shared_ptr<T>>(std::atomic_load(&_listener),
                               method,
                               std::forward<Args>(args)...);
}

template<typename T>
inline Listener<std::shared_ptr<T>>& Listener<std::shared_ptr<T>>::
    operator = (Listener&& tmp) noexcept
{
    if (&tmp != this) {
        std::atomic_exchange(&_listener, std::move(tmp._listener));
    }
    return *this;
}

template<typename T>
inline Listener<std::shared_ptr<T>>& Listener<std::shared_ptr<T>>::
    operator = (std::shared_ptr<T> listener) noexcept
{
    set(std::move(listener));
    return *this;
}

} // namespace Bricks
