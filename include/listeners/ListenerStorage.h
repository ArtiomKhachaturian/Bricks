// Copyright 2026 Artiom Khachaturian
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
#pragma once // ListenerStorage.h
#include "threadsafety/SafeObjAliases.h"
#include "Invoke.h"
#include <atomic>
#include <memory>

namespace Bricks::internal
{

// T is shared_ptr/unique_ptr/weak_ptr/T*
template <typename T, bool ThreadSafe> class ListenerStorage;

template <typename T> class ListenerStorage<std::shared_ptr<T>, true> {
public:
    ListenerStorage() = default;
    ListenerStorage(std::shared_ptr<T> u) : _u{std::move(u)} {}
    template <typename U>
    ListenerStorage(U u) : ListenerStorage(std::shared_ptr<T>(std::move(u))) {}
    ListenerStorage(const ListenerStorage& src) { assign(src); }
    ListenerStorage(ListenerStorage&& tmp) noexcept { assign(std::move(tmp)); };
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            assign(src);
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            assign(std::move(tmp));
        }
        return *this;
    }
    void set(std::shared_ptr<T> u) {
        std::atomic_store_explicit(&_u, std::move(u), std::memory_order_release);
    };
    template <typename U>
    void set(U u) {
        set(std::shared_ptr<T>(std::move(u)));
    }
    std::shared_ptr<T> get() const noexcept {
        return std::atomic_load_explicit(&_u, std::memory_order_acquire);
    }
    bool empty() const noexcept { return nullptr == get(); }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        Invoke<std::shared_ptr<T>>::make(get(), method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        return Invoke<std::shared_ptr<T>, R>::makeR(get(), method, std::forward<Args>(args)...);
    }
private:
    void assign(const ListenerStorage& src) {
        if (&src != this) {
            // CAS
            auto new_ptr = std::atomic_load_explicit(&src._u, std::memory_order_acquire);
            auto current = std::atomic_load_explicit(&_u, std::memory_order_relaxed);
            while (!std::atomic_compare_exchange_weak_explicit(&_u,
                                                               &current,
                                                               new_ptr,
                                                               std::memory_order_release,
                                                               std::memory_order_relaxed)) {
                // reload new version from src
                new_ptr = std::atomic_load_explicit(&src._u, std::memory_order_acquire);
            }
        }
    }
    void assign(ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            auto new_ptr = std::atomic_exchange_explicit(&tmp._u,
                                                         std::shared_ptr<T>{},
                                                         std::memory_order_acq_rel);
            std::atomic_store_explicit(&_u, std::move(new_ptr), std::memory_order_release);
        }
    }
private:
    std::shared_ptr<T> _u = {};
};

template <typename T> class ListenerStorage<std::shared_ptr<T>, false> {
public:
    ListenerStorage() = default;
    ListenerStorage(std::shared_ptr<T> u) : _u{std::move(u)} {}
    template <typename U>
    ListenerStorage(U u) : ListenerStorage(std::shared_ptr<T>(std::move(u))) {}
    ListenerStorage(const ListenerStorage& src) : _u{src._u} {}
    ListenerStorage(ListenerStorage&& tmp) noexcept : _u{std::move(tmp._u)} {}
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            _u = src._u;
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            _u = std::move(tmp._u);
        }
        return *this;
    }
    void set(std::shared_ptr<T> u) { _u = std::move(u); };
    template <typename U>
    void set(U u) {
        set(std::shared_ptr<T>(std::move(u)));
    }
    const std::shared_ptr<T>& get() const noexcept { return _u; }
    bool empty() const noexcept { return nullptr == _u; }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        Invoke<std::shared_ptr<T>>::make(_u, method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        return Invoke<std::shared_ptr<T>, R>::makeR(_u, method, std::forward<Args>(args)...);
    }
private:
    std::shared_ptr<T> _u = {};
};

template <typename T, typename Deleter, bool ThreadSafe>
class ListenerStorage<std::unique_ptr<T, Deleter>, ThreadSafe> {
public:
    ListenerStorage() = default;
    ListenerStorage(std::unique_ptr<T, Deleter> u) : _s{std::move(u)} {}
    ListenerStorage(const ListenerStorage& src) : _s{src._s} {}
    ListenerStorage(ListenerStorage&& tmp) noexcept : _s{std::move(tmp._s)} {}
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            _s = src._s;
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            _s = std::move(tmp._s);
        }
        return *this;
    }
    auto get() const noexcept { return _s.get(); }
    void set(std::unique_ptr<T, Deleter> u) { _s.set(std::move(u)); };
    bool empty() const noexcept { return _s.empty(); }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        _s.invoke(method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        return _s.template invokeR<R>(method, std::forward<Args>(args)...);
    }
private:
    ListenerStorage<std::shared_ptr<T>, ThreadSafe> _s;
};

template <typename T> class ListenerStorage<T*, true> {
public:
    ListenerStorage() = default;
    ListenerStorage(T* u) : _u{u} {}
    ListenerStorage(const ListenerStorage& src) { assign(src); }
    ListenerStorage(ListenerStorage&& tmp) noexcept { assign(std::move(tmp)); }
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            assign(src);
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            assign(std::move(tmp));
        }
        return *this;
    }
    void set(T* u) { _u(u); };
    bool empty() const noexcept {
        LOCK_READ_SAFE_OBJ(_u);
        return nullptr == _u.constRef();
    }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        LOCK_READ_SAFE_OBJ(_u);
        Invoke<T*>::make(_u.constRef(), method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        LOCK_READ_SAFE_OBJ(_u);
        return Invoke<T*, R>::makeR(_u.constRef(), method, std::forward<Args>(args)...);
    }
private:
    void assign(const ListenerStorage& src) {
        if (&src != this) {
            _u(src._u());
        }
    }
    void assign(ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            LOCK_WRITE_SAFE_OBJ(tmp._u);
            _u(tmp._u.take());
        }
    }
private:
    SafeObj<T*, std::recursive_mutex> _u = {};
};

template <typename T> class ListenerStorage<T*, false> {
public:
    ListenerStorage() = default;
    ListenerStorage(T* u) : _u{u} {}
    ListenerStorage(const ListenerStorage& src) : _u{src._u} {}
    ListenerStorage(ListenerStorage&& tmp) noexcept {
        _u = tmp._u;
        tmp._u = nullptr;
    }
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            _u = src._u;
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            _u = tmp._u;
            tmp._u = nullptr;
        }
        return *this;
    }
    void set(T* u) { _u = u;};
    bool empty() const noexcept { return _u == nullptr; }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        Invoke<T*>::make(_u, method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        return Invoke<T*, R>::makeR(_u, method, std::forward<Args>(args)...);
    }
private:
    T* _u = nullptr;
};

template <typename T> class ListenerStorage<std::weak_ptr<T>, true> {
public:
    ListenerStorage() = default;
    ListenerStorage(std::weak_ptr<T> u) : _u{std::move(u)} {}
    ListenerStorage(const ListenerStorage& src) { assign(src); }
    ListenerStorage(ListenerStorage&& tmp) noexcept { assign(std::move(tmp)); }
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            assign(src);
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            assign(std::move(tmp));
        }
        return *this;
    }
    void set(std::weak_ptr<T> u) { _u(std::move(u)); };
    bool empty() const noexcept {
        LOCK_READ_SAFE_OBJ(_u);
        return _u->expired();
    }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        Invoke<std::weak_ptr<T>>::make(get(), method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        return Invoke<std::weak_ptr<T>, R>::makeR(get(), method, std::forward<Args>(args)...);
    }
private:
    auto get() const noexcept { return _u(); }
    void assign(const ListenerStorage& src) {
        if (&src != this) {
            _u(src._u());
        }
    }
    void assign(ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            LOCK_WRITE_SAFE_OBJ(tmp._u);
            _u(tmp._u.take());
        }
    }
private:
    SafeWeakPtr<T, std::mutex> _u;
};

template <typename T> class ListenerStorage<std::weak_ptr<T>, false> {
public:
    ListenerStorage() = default;
    ListenerStorage(std::weak_ptr<T> u) : _u{std::move(u)} {}
    ListenerStorage(const ListenerStorage& src) : _u{src._u} {}
    ListenerStorage(ListenerStorage&& tmp) noexcept {
        _u = tmp._u;
        tmp._u = {};
    }
    ListenerStorage& operator = (const ListenerStorage& src) {
        if (&src != this) {
            _u = src._u;
        }
        return *this;
    }
    ListenerStorage& operator = (ListenerStorage&& tmp) noexcept {
        if (&tmp != this) {
            _u = tmp._u;
            tmp._u = {};
        }
        return *this;
    }
    void set(std::weak_ptr<T> u) { _u = std::move(u); };
    bool empty() const noexcept { return _u.expired(); }
    template <class Method, typename... Args>
    void invoke(const Method& method, Args&&... args) const {
        Invoke<std::weak_ptr<T>>::make(_u, method, std::forward<Args>(args)...);
    }
    template <typename R, class Method, typename... Args>
    R invokeR(const Method& method, Args&&... args) const {
        return Invoke<std::weak_ptr<T>, R>::makeR(_u, method, std::forward<Args>(args)...);
    }
private:
    std::weak_ptr<T> _u;
};

} // namespace Bricks
