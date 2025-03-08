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
template<typename T>
class Listener
{
public:
    Listener() = default;
    constexpr Listener(std::nullptr_t) noexcept {}
    explicit Listener(T listener);
    Listener(Listener&& tmp) noexcept;
    Listener(const Listener&) = delete;
    ~Listener() { reset(); }
    template <typename U = T>
    void set(U listener = {});
    void reset() { set({}); }
    bool empty() const noexcept;
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const;
    Listener& operator = (const Listener&) = delete;
    Listener& operator = (Listener&& tmp) noexcept;
    template <typename U = T>
    Listener& operator = (U listener) noexcept;
    explicit operator bool() const noexcept { return !empty(); }
private:
    SafeObj<T, std::recursive_mutex> _listener;
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
    explicit operator bool() const noexcept { return !empty(); }
private:
    std::shared_ptr<T> _listener;
};

template<typename T>
inline Listener<T>::Listener(T listener)
    : _listener(std::move(listener))
{
}

template<typename T>
inline Listener<T>::Listener(Listener&& tmp) noexcept
{
    _listener = tmp._listener.take();
}

template<typename T> template<typename U>
inline void Listener<T>::set(U listener)
{
    LOCK_WRITE_SAFE_OBJ(_listener);
    _listener = std::move(listener);
}

template<typename T>
inline bool Listener<T>::empty() const noexcept
{
    LOCK_READ_SAFE_OBJ(_listener);
    return nullptr == _listener.constRef();
}

template<typename T>
template <class Method, typename... Args>
inline void Listener<T>::invoke(const Method& method, Args&&... args) const
{
    LOCK_READ_SAFE_OBJ(_listener);
    Invoke<T>::make(_listener.constRef(), method, std::forward<Args>(args)...);
}

template<typename T>
inline Listener<T>& Listener<T>::operator = (Listener&& tmp) noexcept
{
    if (&tmp != this) {
        set(tmp._listener.take());
    }
    return *this;
}

template<typename T> template<typename U>
inline Listener<T>& Listener<T>::operator = (U listener) noexcept
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
    Invoke<std::shared_ptr<T>>::make(std::atomic_load(&_listener),
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
